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
  
/* PATH_MAX and MAX_PATH are all not quite reliable. Currently the NTFS
 * seems having the longest path limit of 32768 utf-16 */
#define RNM_PATH_MAX    65536
/*#ifdef        CFG_UNIX_API
#include <limits.h>
#define RNM_PATH_MAX    (PATH_MAX << 1)
#else
#define RNM_PATH_MAX    (MAX_PATH << 2)
#endif*/



static int rename_recursive(RNOPT *opt, char *path);
static int rename_recursive_cb(void *option, char *path, int type, void *info);
static int rename_action(RNOPT *opt, char *oldname);
static int rename_prompt(RNOPT *opt, char *fname);
static int match_regexpr(RNOPT *opt, char *fname, int flen);
static int match_forward(RNOPT *opt, char *fname, int flen);
static int match_backward(RNOPT *opt, char *fname, int flen);
static int match_suffix(RNOPT *opt, char *fname, int flen);
static int postproc_prefix(RNOPT *opt, char *fname, int flen);
static int postproc_suffix(RNOPT *opt, char *fname, int flen);
static int postproc_lowercase(unsigned char *s);
static int postproc_uppercase(unsigned char *s);
static int inject(char *rec, int rlen, int del, int room, char *in, int ilen);
static int console_notify(RNOPT *opt, int msg, int v, void *dest, void *sour);



int rename_enfile(RNOPT *opt, char *filename)
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

int rename_entry(RNOPT *opt, char *filename)
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

int rename_notify(RNOPT *opt, int msg, int v, void *dest, void *sour)
{
	int	rc;

	if ((opt == NULL) || (opt->notify == NULL)) {
		return console_notify(opt, msg, v, dest, sour);
	}
	rc = opt->notify(opt, msg, v, dest, sour);
	if (rc == RNM_ERR_EVENT) {
		return console_notify(opt, msg, v, dest, sour);
	}
	return rc;
}

static int rename_recursive(RNOPT *opt, char *path)
{
	return smm_pathtrek(path, SMM_PATH_DIR_FIFO, rename_recursive_cb, opt);
}

static int rename_recursive_cb(void *option, char *path, int type, void *info)
{
	RNOPT	*opt = option;
	int	rc = RNM_ERR_NONE;

	(void) info;
	switch (type) {
	case SMM_MSG_PATH_ENTER:
		rename_notify(opt, RNM_MSG_ENTER_DIR, 0, path, NULL);
		break;
	case SMM_MSG_PATH_LEAVE:
		rename_notify(opt, RNM_MSG_LEAVE_DIR, 0, path, NULL);
		break;
	case SMM_MSG_PATH_EXEC:
		rc = rename_action(opt, path);
		break;
	}
	return rc;
}

static int rename_action(RNOPT *opt, char *oldname)
{
	int	rc;

	if ((opt->buffer = rename_alloc(opt, oldname)) == NULL) {
		return RNM_ERR_RENAME;
	}
	rc = rename_executing(opt, opt->buffer, oldname);
	opt->buffer = smm_free(opt->buffer);
	return rc;
}

int rename_executing(RNOPT *opt, char *dest, char *sour)
{
	int	rc;

	if (smm_fstat(dest) >= 0) {	/* the target file has existed already */
		switch (opt->cflags & RNM_CFLAG_PROMPT_MASK) {
		case RNM_CFLAG_NEVER:
			rename_notify(opt, RNM_MSG_SKIP_EXISTED, 0, dest, sour);
			return RNM_ERR_NONE;
		case RNM_CFLAG_ALWAYS:
			rename_notify(opt, RNM_MSG_OVERWRITE, 0, dest, sour);
			break;
		default:
			rc = rename_notify(opt, RNM_MSG_PROMPT, 0, dest, sour);
			if (rc == RNM_ERR_SKIP) {
				return RNM_ERR_NONE;
			}
			break;
		}
	}
	if (opt->cflags & RNM_CFLAG_TEST) {
		rename_notify(opt, RNM_MSG_SIMULATION, 0, dest, sour);
		return RNM_ERR_NONE;
	}
	if ((rc = rename(sour, dest)) < 0) {
		rename_notify(opt, RNM_MSG_SYS_FAIL, rc, dest, sour);
		return RNM_ERR_RENAME;
	}
	opt->rpcnt++;
	rename_notify(opt, RNM_MSG_RENAME, opt->rpcnt, dest, sour);
	return RNM_ERR_NONE;
}

static int rename_prompt(RNOPT *opt, char *fname)
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

static int console_notify(RNOPT *opt, int msg, int v, void *a1, void *a2)
{
	char	*dest = a1, *sour = a2;

	(void) v;

	switch (msg) {
	case RNM_MSG_ENTER_DIR:
		if (opt->cflags & RNM_CFLAG_VERBOSE) {
			printf("Entering directory [%s]\n", dest);
		}
		break;
	case RNM_MSG_LEAVE_DIR:
		if (opt->cflags & RNM_CFLAG_VERBOSE) {
			printf("Leaving directory [%s]\n", dest);
		}
		break;
	case RNM_MSG_ACT_FORWARD:
	case RNM_MSG_ACT_BACKWARD:
	case RNM_MSG_ACT_REGEX:
	case RNM_MSG_ACT_SUFFIX:
		break;
	case RNM_MSG_PPRO_PREFIX:
	case RNM_MSG_PPRO_SUFFIX:
	case RNM_MSG_PPRO_LOWCASE:
	case RNM_MSG_PPRO_UPCASE:
		break;
	case RNM_MSG_SKIP_EXISTED:
		printf("%s\n  -> %s : skipped\n", sour, dest);
		break;
	case RNM_MSG_OVERWRITE:
		break;
	case RNM_MSG_PROMPT:
		if (rename_prompt(opt, dest) == 0) {
			printf("%s\n  -> %s : skipped\n", sour, dest);
			return RNM_ERR_SKIP;
		}
		break;
	case RNM_MSG_SIMULATION:
		printf("%s\n  -> %s : tested\n", sour, dest);
		break;
	case RNM_MSG_SYS_FAIL:
		perror("rename");
		break;
	case RNM_MSG_RENAME:
		printf("%s\n  -> %s : successful\n", sour, dest);
		break;
	default:
		printf("Unknown message [%d]\n", msg);
		return RNM_ERR_PARAM;
	}
	return RNM_ERR_NONE;
}



/****************************************************************************
 * Core functions of Rename
 ****************************************************************************/
char *rename_alloc(RNOPT *opt, char *oldname)
{
	char	buffer[RNM_PATH_MAX];
	char	*fname;
	int	rc = RNM_ERR_NONE, flen;

	/* indicate the expected filename, not the whole path */
	csc_strlcpy(buffer, oldname, sizeof(buffer));
	fname = csc_path_basename(buffer, NULL, 0);
	opt->room = sizeof(buffer) - strlen(buffer) - 1;

	/* ignore the "." and ".." system path */
	if (!strcmp(fname, ".") || !strcmp(fname, "..")) {
		return NULL;	/* invalided file name */
	}
    
	flen = strlen(fname);
	switch (opt->action)  {
	case RNM_ACT_FORWARD:
		rc = match_forward(opt, fname, flen);
		break;
	case RNM_ACT_BACKWARD:
		rc = match_backward(opt, fname, flen);
		break;
	case RNM_ACT_REGEX:
		rc = match_regexpr(opt, fname, flen);
		break;
	case RNM_ACT_SUFFIX:
		rc = match_suffix(opt, fname, flen);
		break;
	}
	if (rc < 0) {
		return NULL;	/* file name truncated */
	}
	
	if ((opt->oflags & RNM_OFLAG_MASKCASE) == RNM_OFLAG_LOWERCASE) {
		postproc_lowercase((unsigned char *) fname);
	} else if ((opt->oflags & RNM_OFLAG_MASKCASE) == RNM_OFLAG_UPPERCASE) {
		postproc_uppercase((unsigned char *) fname);
	}
	if (opt->oflags & RNM_OFLAG_PREFIX) {
		if (postproc_prefix(opt, fname, flen) < 0) {
			return NULL;	/* file name truncated */
		}
	}
	if (opt->oflags & RNM_OFLAG_SUFFIX) {
		if (postproc_suffix(opt, fname, flen) < 0) {
			return NULL;	/* file name truncated */
		}
	}

	if (!strcmp(buffer, oldname)) {
		return NULL;	/* same name after renaming */
	}

	/* if the destination is directory, which means we must move the
	 * original file into this directory, just like mv(1) does */
	/* mv abc dir/
	 * mv home/path/file home/path/dir == home/path/dir/file */
	//FIXME: DO I need to explicitly write dest path, or system can do itself?
	/*if (smm_fstat(buffer) == SMM_FSTAT_DIR) {
		csc_strlcat(buffer, SMM_DEF_DELIM, sizeof(buffer));
		csc_strlcat(buffer, oldname, sizeof(buffer));
	}*/
	return csc_strcpy_alloc(buffer, 0);
}

/* to match a null-terminated string against the precompiled pattern buffer.
   When successed, it substitutes matches with the second parameter so
   the original string with enough buffer will be modified.
   Note: precompiled pattern buffer be set to globel.
   If no matches in the string, return 0.
*/
static int match_regexpr(RNOPT *opt, char *fname, int flen)
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
		if (opt->rpnum && (count >= opt->rpnum)) {
			break;
		}
	}
	return count;
}

static int match_forward(RNOPT *opt, char *fname, int flen)
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

		if (opt->rpnum && (count >= opt->rpnum)) {
			break;
		}
		fname += opt->su_len;
		flen  -= opt->su_len;
	}
	return count;
}

static int match_backward(RNOPT *opt, char *fname, int flen)
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

		if (opt->rpnum && (count >= opt->rpnum)) {
			break;
		}
		fidx -= opt->pa_len;
		flen += opt->pa_len;
	}
	return count;
}

static int match_suffix(RNOPT *opt, char *fname, int flen)
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

static int postproc_prefix(RNOPT *opt, char *fname, int flen)
{
	if (opt->pre_len < 1) {
		return 0;
	}
	opt->room = inject(fname, flen, 0, opt->room, 
			opt->prefix, opt->pre_len);
	if (opt->room < 0) {
		return opt->room;
	}
	return 1;
}

static int postproc_suffix(RNOPT *opt, char *fname, int flen)
{
	char	*tmp;

	(void) flen;

	if (opt->suf_len < 1) {
		return 0;
	}

	fname = csc_path_basename(fname, NULL, 0);
	if ((tmp = strrchr(fname, '.')) == NULL) {
		tmp = fname;
	}
		
	opt->room = inject(tmp, strlen(tmp), 0, opt->room, 
			opt->suffix, opt->suf_len);
	if (opt->room < 0) {
		return opt->room;
	}
	return 1;
}

static int postproc_lowercase(unsigned char *s)
{
	while (*s) {
		*s = tolower((int) *s);
		s++;
	}
	return 0;
}

static int postproc_uppercase(unsigned char *s)
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


