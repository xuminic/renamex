/*
    rename.c -- file rename tool

    Copyright (C) 1998-2017  "Andy Xuming" <xuming@users.sourceforge.net>

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
#ifdef  HAVE_CONFIG_H
#include <config.h>
#else
#error "Run configure first"
#endif

#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# ifdef HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#ifdef HAVE_STRING_H
# if !defined STDC_HEADERS && defined HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif
#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <ctype.h>

#include "rename.h"

/* re-use the debug protocols in libcsoup */
#define CSOUP_DEBUG_LOCAL       SLOG_CWORD(RENAME_MOD_CORE, SLOG_LVL_WARNING)
#include "libcsoup_debug.h"


static int rename_recursive(RNOPT *opt, char *path);
static int rename_recursive_cb(void *option, char *path, int type, void *info);
static int rename_execute_stage2(RNOPT *opt, char *dest, char *sour);
static int rename_show(char *dest, char *sour, char *action);
static int console_notify(RNOPT *opt, int msg, int v, void *dest, void *sour);
static int match_regexpr(RNOPT *opt, char *fname, int flen);
static int match_forward(RNOPT *opt, char *fname, int flen);
static int match_backward(RNOPT *opt, char *fname, int flen);
static int match_extension(RNOPT *opt, char *fname, int flen);
static int postproc_prefix(RNOPT *opt, char *fname, int flen);
static int postproc_suffix(RNOPT *opt, char *fname, int flen);
static int postproc_lowercase(unsigned char *s);
static int postproc_uppercase(unsigned char *s);
static int inject(char *rec, int rlen, int del, int room, char *in, int ilen);



int rename_enfile(RNOPT *opt, char *filename)
{
	FILE	*fp;
	char	buf[RNM_PATH_MAX];

	if ((fp = fopen(filename, "r")) == NULL) {
		return RNM_ERR_OPENFILE;
	}
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		buf[strlen(buf)-1] = 0;
		rename_entry(opt, buf);
	}
	fclose(fp);
	return RNM_ERR_NONE;
}

int rename_entry(RNOPT *opt, char *filename)
{
	int	rc;

	if (opt->cflags & RNM_CFLAG_RECUR)  {
		if (smm_fstat(filename) == SMM_FSTAT_DIR) {
			rename_recursive(opt, filename);
		}
	}
	if ((rc = rename_open_buffer(opt, filename)) == RNM_ERR_NONE) {
		rc = rename_executing(opt, opt->buffer, filename);
	}
	return rc;
}

int rename_executing(RNOPT *opt, char *dest, char *sour)
{
	char	*bname;
	int	rc;

	opt->st_process++;

	if (!strcmp(dest, sour)) {
		rc = RNM_ERR_IGNORE;	/* ignore the same name */
	} else if (smm_fstat(dest) != SMM_FSTAT_DIR) {
		rc = rename_execute_stage2(opt, dest, sour);
	} else {
		/* the destination is directory, which means the source file
		 * must be moved to that directory, just like mv(1) does. i.g
		 *   mv home/path/file home/path/dir == home/path/dir/file 
		 * This process must explicitly manipulate the path
		 * because the system call rename(2) won't do as mv(1) */
		bname = csc_path_basename(sour, NULL, 0);
		if (*bname == 0) {
			opt->st_failed++;
			return RNM_ERR_PARAM;
		}
		dest = csc_strcpy_alloc(dest, strlen(bname) + 8);
		if (dest == NULL) {
			opt->st_failed++;
			return RNM_ERR_LOWMEM;
		}

		strcat(dest, SMM_DEF_DELIM);
		strcat(dest, bname);
		rc = rename_execute_stage2(opt, dest, sour);
		smm_free(dest);
	}
	switch (rc) {
	case RNM_ERR_NONE:
		opt->st_success++;
		rename_notify(opt, RNM_MSG_RENAME, 
				opt->st_success, dest, sour);
		break;
	case RNM_ERR_RENAME:
		opt->st_failed++;
		break;
	case RNM_ERR_IGNORE:
		opt->st_same++;
		break;
	case RNM_ERR_SKIP:
		opt->st_skip++;
	}
	return rc;
}

int rename_status_clean(RNOPT *opt)
{
	opt->st_process = 0;
	opt->st_success = 0;
	opt->st_failed = 0;
	opt->st_same = 0;
	opt->st_skip = 0;
	return 0;
}

int rename_option_dump(RNOPT *opt)
{
	char	buf[80];

	switch (opt->cflags & RNM_CFLAG_PROMPT_MASK) {
	case RNM_CFLAG_NEVER:
		strcpy(buf, "[SKIP");
		break;
	case RNM_CFLAG_ALWAYS:
		strcpy(buf, "[OVERWT");
		break;
	default:
		strcpy(buf, "[AUTO");
		break;
	}
	if (opt->cflags & RNM_CFLAG_RECUR) {
		strcat(buf, "|RECUR");
	}
	if (opt->cflags & RNM_CFLAG_VERBOSE) {
		strcat(buf, "|VERBOSE");
	}
	if (opt->cflags & RNM_CFLAG_TEST) {
		strcat(buf, "|TEST");
	}
	if (opt->cflags & RNM_CFLAG_GUI) {
		strcat(buf, "|GUI");
	}
	CDB_SHOW(("Control Flags:  %s]\n", buf));

	strcpy(buf, "[");
	switch (opt->oflags & RNM_OFLAG_MASKCASE) {
	case RNM_OFLAG_LOWERCASE:
		strcat(buf, "LOWCASE");
		break;
	case RNM_OFLAG_UPPERCASE:
		strcat(buf, "UPCASE");
		break;
	}
	if (opt->oflags & RNM_OFLAG_PREFIX) {
		strcat(buf, "|PREFIX");
	}
	if (opt->oflags & RNM_OFLAG_SUFFIX) {
		strcat(buf, "|SUFFIX");
	}
	switch (opt->action) {
	case RNM_ACT_FORWARD:
		strcat(buf, "][FORWARD");
		break;
	case RNM_ACT_BACKWARD:
		strcat(buf, "][BACKWARD");
		break;
	case RNM_ACT_REGEX:
		strcat(buf, "][REGEX");
		break;
	case RNM_ACT_EXTENSION:
		strcat(buf, "][EXT");
		break;
	default:
		strcat(buf, "][NONE");
		break;
	}
	if (opt->regflag & REG_ICASE) {
		strcat(buf, "|ICASE");
	}
	if (opt->regflag & REG_EXTENDED) {
		strcat(buf, "|XREGEX");
	}
	strcat(buf, "]");
	CDB_SHOW(("Process Flags:  %s\n", buf));

	CDB_SHOW(("Pattern:        %s (%d)\n", opt->pattern, opt->pa_len));
	CDB_SHOW(("Substituter:    %s (%d)(x %d)\n", 
			opt->substit, opt->su_len, opt->rpnum));
	if (opt->oflags & RNM_OFLAG_PREFIX) {
		CDB_SHOW(("Prefix:         %s (%d)\n", 
				opt->prefix, opt->pre_len));
	}
	if (opt->oflags & RNM_OFLAG_SUFFIX) {
		CDB_SHOW(("Suffix:         %s (%d)\n", 
				opt->suffix, opt->suf_len));
	}
	CDB_SHOW(("\n"));
	return 0;
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

	(void) info;
	switch (type) {
	case SMM_MSG_PATH_ENTER:
		rename_notify(opt, RNM_MSG_ENTER_DIR, 0, path, NULL);
		break;
	case SMM_MSG_PATH_LEAVE:
		rename_notify(opt, RNM_MSG_LEAVE_DIR, 0, path, NULL);
		break;
	case SMM_MSG_PATH_EXEC:
		if (rename_open_buffer(opt, path) == RNM_ERR_NONE) {
			rename_executing(opt, opt->buffer, path);
		}
		break;
	}
	return SMM_NTF_PATH_NONE;
}

static int rename_execute_stage2(RNOPT *opt, char *dest, char *sour)
{
	int	rc;

	/* test if the target file has existed already.
	 * using smm_fncmp() to seperate the difference between Windows and
	 * unix because Windows doesn't care the capitcal or small letters */
	if ((smm_fstat(dest) >= 0) && smm_fncmp(dest, sour)) {  
		/* the target file has existed already */
		/* Updated 20160717: remove the overwrite option totally.
		 * You don't want anything being accidently overwritten by 
		 * a rename tool. If you want, you need to use other tools. */
		/*
		switch (opt->cflags & RNM_CFLAG_PROMPT_MASK) {
		case RNM_CFLAG_NEVER:
			rename_notify(opt, RNM_MSG_SKIP_EXISTED, 0, dest,sour);
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
		}*/
		rename_notify(opt, RNM_MSG_SKIP_EXISTED, 0, dest,sour);
		return RNM_ERR_SKIP;
	}
	if (opt->cflags & RNM_CFLAG_TEST) {
		rename_notify(opt, RNM_MSG_SIMULATION, 0, dest, sour);
		return RNM_ERR_NONE;
	}
	if ((rc = smm_rename(sour, dest)) < 0) {
		rename_notify(opt, RNM_MSG_SYS_FAIL, rc, dest, sour);
		return RNM_ERR_RENAME;
	}
	return RNM_ERR_NONE;
}

static int rename_show(char *dest, char *sour, char *action)
{
	(void) dest; (void) sour; (void) action;

#ifdef	DEBUG
	CDB_INFO(("renaming: %s\n", sour));
	CDB_INFO(("     -->  %s : %s\n", dest, action));
#else
	printf("renaming: %s\n", sour);
	printf("     -->  %s : %s\n", dest, action);
#endif
	return 0;
}

static int console_notify(RNOPT *opt, int msg, int v, void *a1, void *a2)
{
	char	*dest = a1, *sour = a2;

	(void) v;

	switch (msg) {
	case RNM_MSG_ENTER_DIR:
		/*if (opt->cflags & RNM_CFLAG_VERBOSE) {
			printf("Entering directory [%s]\n", dest);
		}*/
		CDB_INFO(("Entering directory [%s]\n", dest));
		break;
	case RNM_MSG_LEAVE_DIR:
		/*if (opt->cflags & RNM_CFLAG_VERBOSE) {
			printf("Leaving directory [%s]\n", dest);
		}*/
		CDB_INFO(("Leaving directory [%s]\n", dest));
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
		if (opt->cflags & RNM_CFLAG_VERBOSE) {
			rename_show(dest, sour, "skipped");
		}
		break;
	case RNM_MSG_OVERWRITE:
		break;
	case RNM_MSG_PROMPT:
		return RNM_ERR_SKIP;
	case RNM_MSG_SIMULATION:
		rename_show(dest, sour, "tested");
		break;
	case RNM_MSG_SYS_FAIL:
		perror("rename");
		break;
	case RNM_MSG_RENAME:
		if (opt->cflags & RNM_CFLAG_VERBOSE) {
			rename_show(dest, sour, "successful");
		}
		break;
	default:
		CDB_ERROR(("Unknown message [%d]\n", msg));
		return RNM_ERR_PARAM;
	}
	return RNM_ERR_NONE;
}



/****************************************************************************
 * Core functions of Rename
 ****************************************************************************/
int rename_open_buffer(RNOPT *opt, char *oldname)
{
	char	*fname;
	int	rc = RNM_ERR_NONE, flen;

	/* indicate the expected filename, not the whole path */
	csc_strlcpy(opt->buffer, oldname, RNM_PATH_MAX);
	fname = csc_path_basename(opt->buffer, NULL, 0);
	opt->room = RNM_PATH_MAX - strlen(opt->buffer) - 1;

	/* ignore the "." and ".." system path */
	if (!strcmp(fname, ".") || !strcmp(fname, "..")) {
		return RNM_ERR_OPENFILE;	/* invalided file name */
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
	case RNM_ACT_EXTENSION:
		rc = match_extension(opt, fname, flen);
		break;
	}
	if (rc < 0) {
rename_open_buffer_error:
		CDB_ERROR(("rename_open_buffer: file name truncated\n"));
		return RNM_ERR_LONGPATH;	/* file name truncated */
	}
	
	if ((opt->oflags & RNM_OFLAG_MASKCASE) == RNM_OFLAG_LOWERCASE) {
		postproc_lowercase((unsigned char *) fname);
	} else if ((opt->oflags & RNM_OFLAG_MASKCASE) == RNM_OFLAG_UPPERCASE) {
		postproc_uppercase((unsigned char *) fname);
	}
	if (opt->oflags & RNM_OFLAG_PREFIX) {
		if (postproc_prefix(opt, fname, flen) < 0) {
			goto rename_open_buffer_error;
		}
	}
	if (opt->oflags & RNM_OFLAG_SUFFIX) {
		if (postproc_suffix(opt, fname, flen) < 0) {
			goto rename_open_buffer_error;
		}
	}
	return RNM_ERR_NONE;
}

/* 20160712: testing or compiling error: 
 *   "-s *sfdg/dfg/x"  or "-s/\(sfdg/dfg/x"
 * Note that extended RE only. */
int rename_compile_regex(RNOPT *opt)
{
	char	errmsg[8192];
	int	rcode;

	if (opt->action != RNM_ACT_REGEX) {
		return 0; /* it's ok being not regular expression */
	}
	rcode = regcomp(opt->preg, opt->pattern, opt->regflag);
	if (rcode != 0) {
		regerror(rcode, opt->preg, errmsg, sizeof(errmsg));
		CDB_ERROR(("Regular Expression '%s': %s\n", 
				opt->pattern, errmsg));
	}
	return rcode;
}

/* to match a null-terminated string against the precompiled pattern buffer.
   When successed, it substitutes matches with the second parameter so
   the original string with enough buffer will be modified.
   Note: precompiled pattern buffer be set to globel.
   If no matches in the string, return 0.
   A good start of regex: 8 Regular Expressions You Should Know
   http://code.tutsplus.com/tutorials/8-regular-expressions-you-should-know--net-6149
*/
static int match_regexpr(RNOPT *opt, char *fname, int flen)
{
	regmatch_t	pmatch[1];
	int		count = 0;

#ifdef	DEBUG
	void dump_matched_regexpr(regmatch_t *pm)
	{
		char	buf[256];
		int	i, k;

		sprintf(buf, "match_regexpr[%d-%d]: ", pm->rm_so, pm->rm_eo);
		for (i = pm->rm_so, k = strlen(buf); 
				(i < pm->rm_eo) && (k < 255); ) {
			buf[k++] = fname[i++];
		}
		buf[k++] = 0;
		CDB_FUNC(("%s\n", buf));
	}
#endif
	while (!regexec(opt->preg, fname, 1, pmatch, 0))  {
#ifdef	DEBUG
		dump_matched_regexpr(pmatch);
#endif
		/* 20160712 for unknown reason the '-s/e?/-/xg' option crash
		 * the regexec() by rm_so == rm_eo == 0 */
		if (pmatch->rm_so >= pmatch->rm_eo) {
			break;
		}

		opt->room = inject(fname+pmatch->rm_so, flen - pmatch->rm_so,
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

static int match_extension(RNOPT *opt, char *fname, int flen)
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

	/* suffix goes to beween file name and extension name */
	if ((tmp = strrchr(fname, '.')) == NULL) {
		tmp = fname + strlen(fname);
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


