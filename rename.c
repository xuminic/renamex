/*
    rename.c -- file rename tool

    Copyright (C) 1998-2011  "Andy Xuming" <xuming@users.sourceforge.net>

    This file is part of RENAME, a utility to help file renaming

    RENAME is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    RENAME is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ctype.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <termios.h>
#include <sys/stat.h>

#if HAVE_UNISTD_H
  #include <sys/types.h>
  #include <unistd.h>
#endif

#if STDC_HEADERS
  #include <string.h>
#else
  #ifndef HAVE_STRCHR
    #define strchr index
    #define strrchr rindex
  #endif
  char *strchr(), *strrchr();
#endif

#if HAVE_DIRENT_H
  #include <dirent.h>
  #define NAMLEN(dirent) strlen((dirent)->d_name)
#else
  #define dirent direct
  #define NAMLEN(dirent) (dirent)->d_namlen
  #if HAVE_SYS_NDIR_H
    #include <sys/ndir.h>
  #endif
  #if HAVE_SYS_DIR_H
    #include <sys/dir.h>
  #endif
  #if HAVE_NDIR_H
    #include <ndir.h>
  #endif
#endif

#if HAVE_REGEX_H
  #include <regex.h>
#else
  #include "regex.h"
#endif

#include "libcsoup.h"
#include "rename.h"
  

static	char	*rep_state[] = { "done", "skip", "test", "fail", "own" };

static int rename_recursive(RENOP *opt, char *path);
static int rename_recursive_cb(void *option, char *path, int type, void *info);
static int rename_action(RENOP *opt, char *oldname);
static int rename_executing(RENOP *opt, char *dest, char *sour);
static int rename_prompt(RENOP *opt, char *fname);
#ifdef	CFG_REGEX
static int match_regexpr(RENOP *opt, char *fname, int flen);
#endif
static int match_forward(RENOP *opt, char *fname, int flen);
static int match_backward(RENOP *opt, char *fname, int flen);
static int match_suffix(RENOP *opt, char *fname, int flen);
static int match_lowercase(unsigned char *s);
static int match_uppercase(unsigned char *s);
static int inject(char *rec, int rlen, int del, int room, char *in, int ilen);
static int report(char *dest, char *sour, int state, int flag);



int rename_enfile(RENOP *opt, char *filename)
{
	FILE	*fp;
	char	buf[RNM_PATH_MAX];
	int	rc = RNM_ERR_NONE;

	if ((fp = fopen(filename, "r")) == NULL) {
		return RNM_ERR_OPENFILE;
	}
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		buf[strlen(buf)-1] = 0;
		rc = rename_entry(opt, buf);
		if (rc != RNM_ERR_NONE) {
			break;
		}
	}
	fclose(fp);
	return rc;
}

int rename_entry(RENOP *opt, char *filename)
{
	int	rc;

	if (opt->cflags & RNM_CFLAG_RECUR)  {
		if (smm_fstat(filename) == SMM_FSTAT_DIR) {
			rc = rename_recursive(opt, filename);
			if (rc != RNM_ERR_NONE) {
				return rc;	//FIXME: return to entry dir
			}
		}
	}
	return rename_action(opt, filename);
}

static int rename_recursive(RENOP *opt, char *path)
{
	return smm_pathtrek(path, SMM_PATH_DIR_FIFO, rename_recursive_cb, opt);
}

static int rename_recursive_cb(void *option, char *path, int type, void *info)
{
	RENOP	*opt = option;
	int	rc = RNM_ERR_NONE;

	(void) info;
	switch (type) {
	case SMM_MSG_PATH_ENTER:
		if (opt->cflags & RNM_CFLAG_VERBOSE) {
			printf("Entering directory [%s]\n", path);
		}
		break;
	case SMM_MSG_PATH_LEAVE:
		if (opt->cflags & RNM_CFLAG_VERBOSE) {
			printf("Leaving directory [%s]\n", path);
		}
		break;
	case SMM_MSG_PATH_EXEC:
		rc = rename_action(opt, path);
		break;
	}
	return rc;
}

static int rename_action(RENOP *opt, char *oldname)
{
	char	*fname;
	int	rc = RNM_ERR_NONE, flen, renamed = 0;

	if ((opt->buffer = csc_strcpy_alloc(oldname, RNM_PATH_MAX)) == NULL) {
		return RNM_ERR_LOWMEM;
	}
	opt->room = RNM_PATH_MAX;

	/* indicate the expected filename, not the whole path */
	fname = csc_path_basename(opt->buffer, NULL, 0);
	
	if (!strcmp(fname, ".") || !strcmp(fname, "..")) {
		opt->buffer = smm_free(opt->buffer);
		return RNM_ERR_NONE;
	}
    
	flen = strlen(fname);
	switch (opt->action)  {
	case RNM_ACT_FORWARD:
		rc = match_forward(opt, fname, flen);
		break;
	case RNM_ACT_BACKWARD:
		rc = match_backward(opt, fname, flen);
		break;
#ifdef	CFG_REGEX
	case RNM_ACT_REGEX:
		rc = match_regexpr(opt, fname, flen);
		break;
#endif
	case RNM_ACT_SUFFIX:
		rc = match_suffix(opt, fname, flen);
		break;
	}
	if (rc < 0) {
		opt->buffer = smm_free(opt->buffer);
		return RNM_ERR_LONGPATH;
	}
	if (opt->action && !strcmp(opt->buffer, oldname)) {
		opt->buffer = smm_free(opt->buffer);
		return RNM_ERR_NONE;
	}
	
	if ((opt->oflags & RNM_OFLAG_MASKCASE) == RNM_OFLAG_LOWERCASE) {
		match_lowercase((unsigned char *) fname);
	} else if ((opt->oflags & RNM_OFLAG_MASKCASE) == RNM_OFLAG_UPPERCASE) {
		match_uppercase((unsigned char *) fname);
	}

	if (strcmp(opt->buffer, oldname)) {
		rc = rename_executing(opt, opt->buffer, oldname);
		if (rc == RNM_ERR_SKIP) {
			rc = RNM_ERR_NONE;
		} else if (rc == RNM_ERR_NONE) {
			renamed++;
		} else {
			opt->buffer = smm_free(opt->buffer);
			return rc;
		}
	}
	if (renamed) {
		opt->rpcnt++;
	}

	opt->buffer = smm_free(opt->buffer);
	return rc;
}

static int rename_executing(RENOP *opt, char *dest, char *sour)
{
	if (smm_fstat(dest) == SMM_FSTAT_DIR) {
		/* the destination is directory, which means we must move the
		 * original file into this directory, just like mv(1) does */
		if ((int)strlen(sour) + 2 > opt->room) {
			return RNM_ERR_LONGPATH;
		}
		strcat(dest, SMM_DEF_DELIM);
		strcat(dest, sour);
	}
	if (smm_fstat(dest) >= 0) {	/* the target file has existed already */
		switch (opt->cflags & RNM_CFLAG_PROMPT_MASK) {
		case RNM_CFLAG_NEVER:
			report(dest, sour, RNM_REP_SKIP, opt->cflags);
			return RNM_ERR_SKIP;
		case RNM_CFLAG_ALWAYS:
			break;
		default:
			if (rename_prompt(opt, dest) == 0) {
				report(dest, sour, RNM_REP_SKIP, opt->cflags);
				return RNM_ERR_SKIP;
			}
			break;
		}
	}
	if (opt->cflags & RNM_CFLAG_TEST) {
		report(dest, sour, RNM_REP_TEST, opt->cflags);
		return RNM_ERR_SKIP;
	}
	if (rename(sour, dest) < 0) {
		report(dest, sour, RNM_REP_FAILED, opt->cflags);
		return RNM_ERR_RENAME;
	}
	report(dest, sour, RNM_REP_OK, opt->cflags);
	return RNM_ERR_NONE;
}

static int rename_prompt(RENOP *opt, char *fname)
{
	char	buf[64];

	fprintf(stderr, "Overwrite '%s'?  (Yes/No/Always/Skip) ", fname);
	if (fgets(buf, 64, stdin) == NULL) {
		return 0;
	}

	switch (*(csc_strbody(buf, NULL)))  {
	case 'a':
	case 'A':
		opt->cflags &= ~RNM_CFLAG_PROMPT_MASK;
		opt->cflags |= RNM_CFLAG_ALWAYS;
		return 1;
	case 's':
	case 'S':
		opt->cflags &= ~RNM_CFLAG_PROMPT_MASK;
		opt->cflags |= RNM_CFLAG_NEVER;
		return 0;
	case 'y':
	case 'Y':
		return 1;
	case 'n':
	case 'N':
		return 0;
	}
	return 0;
}


/* to match a null-terminated string against the precompiled pattern buffer.
   When successed, it substitutes matches with the second parameter so
   the original string with enough buffer will be modified.
   Note: precompiled pattern buffer be set to globel.
   If no matches in the string, return 0.
*/
#ifdef	CFG_REGEX
static int match_regexpr(RENOP *opt, char *fname, int flen)
{
	regmatch_t	pmatch[1];
	int		count = 0;

	while (!regexec(opt->preg, fname, 1, pmatch, 0))  {
		opt->room = inject(fname + pmatch->rm_so, flen - pmatch->rm_so,
				pmatch->rm_eo - pmatch->rm_so, opt->room,
				opt->substit, opt->su_len);
		if (opt->room < 0) {
			return opt->room;
		}
		fname += pmatch->rm_so + opt->su_len;
		flen  -= pmatch->rm_so - opt->su_len;
		count++;
		
		if (flen <= 0) {
			break;
		}
		if (opt->count && (count >= opt->count)) {
			break;
		}
	}
	return count;
}
#endif

static int match_forward(RENOP *opt, char *fname, int flen)
{
	int	count = 0;

	if (opt->pa_len < 1) {
		return 0;
	}
	while (flen >= opt->pa_len) {
		if (opt->compare(fname, opt->pattern, opt->pa_len)) {
			fname++;
			flen--;
			continue;
		}

		opt->room = inject(fname, flen, opt->pa_len, opt->room,
				opt->substit, opt->su_len);
		if (opt->room < 0) {
			return opt->room;
		}
		count++;
		flen += opt->su_len - opt->pa_len;

		if (opt->count && (count >= opt->count)) {
			break;
		}
		fname += opt->su_len;
		flen  -= opt->su_len;
	}
	return count;
}

static int match_backward(RENOP *opt, char *fname, int flen)
{
	char	*fidx;
	int	count = 0;

	if (opt->pa_len < 1) {
		return 0;
	}

	flen -= opt->pa_len;
	fidx = fname + flen;
	while (fidx >= fname) {
		if (opt->compare(fidx, opt->pattern, opt->pa_len)) {
			fidx--;
			flen++;
			continue;
		}

		opt->room = inject(fidx, flen, opt->pa_len, 
				opt->room, opt->substit, opt->su_len);
		if (opt->room < 0) {
			return opt->room;
		}
		count++;
		flen += opt->su_len - opt->pa_len;

		if (opt->count && (count >= opt->count)) {
			break;
		}
		fidx -= opt->pa_len;
		flen += opt->pa_len;
	}
	return count;
}

static int match_suffix(RENOP *opt, char *fname, int flen)
{
	if (opt->pa_len < 1) {
		return 0;
	}
	if (opt->su_len - opt->pa_len > opt->room) {
		return -1;	/* oversized */
	}
	fname += flen - opt->pa_len;
	if (!opt->compare(fname, opt->pattern, opt->pa_len)) {
		strcpy(fname, opt->substit);
		return 1;
	}
	return 0;
}

static int match_lowercase(unsigned char *s)
{
	while (*s) {
		*s = tolower((int) *s);
		s++;
	}
	return 0;
}

static int match_uppercase(unsigned char *s)
{
	while (*s) {
		*s = toupper((int) *s);
		s++;
	}
	return 0;
}

static int inject(char *rec, int rlen, int del, int room, char *in, int ilen)
{
	char	*sour;
	int	acc;

	sour = rec + del;
	if ((acc = ilen - del) != 0) {
		if ((room -= acc) < 0) {
			return room;
		}
		memmove(sour + acc, sour, rlen - del + 1);
	}
	memcpy(rec, in, ilen);
	return room;
}

static int report(char *dest, char *sour, int state, int flag)
{
#define LISTWIDTH	32
	char	sname[LISTWIDTH], dname[LISTWIDTH];
	int	len;

	if ((flag & RNM_CFLAG_VERBOSE) == 0) {
		return 0;
	}

	memset(dname, ' ', sizeof(dname));
	len = strlen(dest);
	if (len < LISTWIDTH) {
		memcpy(dname, dest, len);
	} else {
		memcpy(dname, dest + len - LISTWIDTH + 1, LISTWIDTH - 1);
		dname[0] = dname[1] = '.';
	}
	dname[LISTWIDTH -1] = 0;

	memset(sname, ' ', sizeof(sname));
	len = strlen(sour);
	if (len < LISTWIDTH) {
		memcpy(sname, sour, len);
	} else {
		memcpy(sname, sour + len - LISTWIDTH + 1, LISTWIDTH - 1);
		sname[0] = sname[1] = '.';
	}
	sname[LISTWIDTH -1] = 0;

	printf("%s -> %s : %s\n", sname, dname, rep_state[state]);
	return 0;
}


