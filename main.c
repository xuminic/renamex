
/*  main.c - command line mode entry

    Copyright (C) 1998-2025  "Andy Xuming" <xuming@users.sourceforge.net>

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
/* Mask matching mode:
 * How to change the file
 *   S01E02 CHS.ass
 * according to the reference name
 *   Season E02 - Roast BasiliskOmeletKakiage.mkv
 * to
 *   Season E02 - Roast BasiliskOmeletKakiage.ass
 *
 * Command line:
 *   -m/S01/CHS -s/mkv/ass *.mkv - *.ass
 *
 * Steps:
 * 1. matching: "S01E02 CHS.ass" with "S01/CHS"
 * 2. masking: S01"E02 "CHS -> "E02 "
 * 3. searching reference name list
 * 4. matching "E02 " in "Season E02 - Roast BasiliskOmeletKakiage.mkv"
 * 5. modify name by "-s/mkv/ass" -> "Season E02 - Roast BasiliskOmeletKakiage.ass"
 * 6. rename: "S01E02 CHS.ass" -> "Season E02 - Roast BasiliskOmeletKakiage.ass"
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
#if	(DEBUG > 0)
#define CSOUP_DEBUG_LOCAL	SLOG_CWORD(RENAME_MOD_CORE, DEBUG)
#elif	defined(DEBUG)
#define CSOUP_DEBUG_LOCAL	SLOG_CWORD(RENAME_MOD_CORE, SLOG_LVL_WARNING)
#endif

#include "libcsoup_debug.h"


static	struct	cliopt	clist[] = {
	{ 0, NULL, 0, "Usage: renamex [OPTIONS] filename ..." },
	{ 0, NULL, 0, "OPTIONS:" },
	{ 'f', "file",      0, "Load file names from the file" },
	{ 'l', "lowercase", 0, "Lowercase the file name" },
	{ 'u', "uppercase", 0, "Uppercase the file name" },
	{ 'p', "prefix",    1, "Add prefix to the file name" },
	{ 'x', "suffix",    1, "Add suffix to the file name" },
	{ 'm', "mask",      1, "Set the matching mask" },
	{ 's', "search",    1, "search and replace in the file name" },
#ifdef	CFG_GUI_ON
	{ 'G', "gui",       0, "start the GUI mode" },
#endif
	{ 'R', "recursive", 0, "Work on files and directories recursively" },
	{ 'v', "verbose",   2, "Display verbose information" },
	{ 't', "test",      0, "Test only mode. Nothing will be changed" },
	{   1, "help",      2, "Display the help message" },
	{   2, "version",   0, "Display the version message" },
	{   3, "debug",     2, "*" },
	{   0, NULL,        0, "\n\
SEARCH & MATCH SETTING:\n\
  -s /PATTERN/STRING[/SW]          Replace the matching PATTERN by STRING.\n\
  -m /PATTERN1/PATTERN2            Set the matching mask.\n\
or\n\
  -s :PATTERN:STRING[:SW]          In MinGW command line.\n\
  -m :PATTERN1:PATTERN2            In MinGW command line.\n\
The SW could be:\n\
  [i] ignore case when searching\n\
  [b] backward searching and replacing\n\
  [e] change file's extension name\n\
  [r] PATTERN is regular expression, see regex(7)\n\
  [x] PATTERN is extended regular expression, see regex(7)\n\
  [g] replace all occurrences in the filename\n\
  [1-9] replace specified occurrences in the filename\n" },
	{ 0, NULL, 0, NULL }
};

const	char	*help_version = "Rename Express " VERSION;

const	char	*help_descript = "\
Rename files by substituting the specified patterns.\n\n\
Copyright (C) 1998-2025 \"Andy Xuming\" <xuming@users.sourceforge.net>\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n";

const	char	*help_credits = "\
The libregex was compiled from source code in MSYS 1.0:\n\
https://sourceforge.net/projects/mingw/files/MSYS/Base/regex\n\
\n\
The icon came from Wikimedia Commons, the free media repository:\n\
https://commons.wikimedia.org/wiki/File:1328101993_Rename.png\n\
\n\
The GUI frontend is based on IUP, a multi-platform toolkit for building\n\
graphical user interfaces.\n\
http://webserver2.tecgraf.puc-rio.br/iup\n";

RNOPT		sysopt;

static int rename_free_all(int sig);
static int cli_set_pattern(RNOPT *opt, char *optarg);
static int cli_set_mask(RNOPT *opt, char *optarg);
static int rename_debug_trans_module(int cw, char *buf, int blen);
static int debug_main(RNOPT *opt, int argc, char **argv);
//static char *strcpy_stop(char *dest, char **sour, int stop);
//static char *strcpy_char(char *dest, char **sour, int c);

int main(int argc, char **argv)
{
	SMMDBG	*dbgc;
	void	*argp;
	int 	c, rc, infile = 0;

	smm_init();
#if	defined(DEBUG) && defined(CFG_WIN32_API) && defined(CFG_GUI_ON)
	//dbgc = slog_csoup_open(NULL, "win32.log");
	dbgc = slog_csoup_open(NULL, NULL);
#else
	dbgc = slog_csoup_open(NULL, NULL);
#endif
	slog_translate_setup(dbgc, SLOG_TRANSL_MODUL, 
			rename_debug_trans_module);

	memset(&sysopt, 0, sizeof(RNOPT));
	sysopt.compare = strncmp;
	sysopt.cflags = RNM_CFLAG_NEVER;

	if ((argp = csc_cli_getopt_open(clist, NULL)) == NULL) {
		slog_csoup_close();
		return -1;
	}
	
#ifdef  CFG_GUI_ON
	if ((sysopt.gui = mmgui_open(&sysopt, &argc, &argv)) == NULL) {
		slog_csoup_close();
		return -2; /* the config file */
	}
#endif
	rc = RNM_ERR_NONE;
	while ((c = csc_cli_getopt(argc, argv, argp)) > 0) {
		switch (c) {
		case 1:
			if (optarg == NULL) {
				csc_cli_print(clist, 0, NULL);
			} else if (!strcmp(optarg, "credit")) {
				puts(help_credits);
			}
			rc = RNM_ERR_HELP;
			break;
		case 2:
			printf("%s, ", help_version);
			puts(help_descript);
			rc = RNM_ERR_HELP;
			break;
		case 3:		/* let "--debug=a b c" equal to "--debug a b c" */
			if (optarg) {	/* like "--debug=a b c" */
				optind--;
			} else {
				optarg = argv[optind];
			}
			if (!strcmp(optarg, "logfile")) {
				slog_bind_file(dbgc, "renamex.log");
			} else {
				debug_main(&sysopt, argc - optind, &argv[optind]);
				rc = RNM_ERR_HELP;
			}
			break;
		case 'f':
			infile = 1;
			break;
		case 'l':
			sysopt.oflags &= ~RNM_OFLAG_MASKCASE;
			sysopt.oflags |= RNM_OFLAG_LOWERCASE;
			break;
		case 'u':
			sysopt.oflags &= ~RNM_OFLAG_MASKCASE;
			sysopt.oflags |= RNM_OFLAG_UPPERCASE;
			break;
		case 'p':
			sysopt.oflags |= RNM_OFLAG_PREFIX;
			sysopt.prefix  = optarg;
			sysopt.pre_len = strlen(optarg);
			break;
		case 'x':
			sysopt.oflags |= RNM_OFLAG_SUFFIX;
			sysopt.suffix  = optarg;
			sysopt.suf_len = strlen(optarg);
			break;
#ifdef	CFG_GUI_ON
		case 'G':
			sysopt.cflags |= RNM_CFLAG_GUI;
			break;
#endif
		case 'R':
			sysopt.cflags |= RNM_CFLAG_RECUR;
			break;
		case 'v':
			sysopt.cflags |= RNM_CFLAG_VERBOSE;
			rc = SLOG_LVL_INFO;
			if (optarg != NULL) {
				rc = (int) strtol(optarg, NULL, 10);
			}
			if ((rc < SLOG_LVL_ERROR) || (rc > SLOG_LVL_FUNC)) {
				rc = SLOG_LVL_INFO;
			}
			CDB_SET_LEVEL(rc);
			rc = RNM_ERR_NONE;
			break;
		case 't':
			//sysopt.cflags |= RNM_CFLAG_TEST | RNM_CFLAG_VERBOSE;
			sysopt.cflags |= RNM_CFLAG_TEST;
			CDB_SET_LEVEL(SLOG_LVL_MODULE);
			break;
		case 's':
			rc = cli_set_pattern(&sysopt, optarg);
			break;
		case 'm':
			rc = cli_set_mask(&sysopt, optarg);
			sysopt.cflags |= RNM_CFLAG_MSKMATCH;
			break;
		}
		if (rc != RNM_ERR_NONE) {
			rename_free_all(0);
			return rc;
		}
	}

	sysopt.rtpath  = smm_cwd_push();
	smm_signal_break(rename_free_all);

#ifdef	CFG_GUI_ON
	if (sysopt.cflags & RNM_CFLAG_GUI) {
		rc = mmgui_run(sysopt.gui, --argc, ++argv);
		rename_free_all(0);
		return rc;
	}
#endif
	if (optind >= argc) {	/* no file name offered */
#ifdef	CFG_GUI_ON
		rc = mmgui_run(sysopt.gui, 0, NULL);
#else
		CDB_SHOW(("renamex: missing file operand\n"));
		rc = RNM_ERR_PARAM;
#endif
		rename_free_all(0);
		return rc;
	}
	if (!sysopt.oflags && !sysopt.action) { /* no operation specified */
		/* if user request command like
		 *   renamex oldname newname
		 * the renamex will do the direct renaming for mv's sake.
		 * However renamex is NOT mv so it won't check the crosslink
		 * of devices or anything fancy */
		if (optind + 2 == argc) {
			rc = rename_executing(&sysopt, 
					argv[optind+1], argv[optind]);
		} else {
#ifdef	CFG_GUI_ON
			rc = mmgui_run(sysopt.gui, --argc, ++argv);
#else
			CDB_SHOW(("renamex: missing rename operand\n"));
			rc = RNM_ERR_PARAM;
#endif
		}
		rename_free_all(0);
		return rc;
	}

	if (rename_compile_regex(&sysopt)) {
		rename_free_all(0);
		return RNM_ERR_REGPAT;
	}

	c = optind;
	if (sysopt.cflags & RNM_CFLAG_MSKMATCH) {
		/* collecting the reference file list for mask matching */
		sysopt.maskref = malloc((argc - optind) * sizeof(char*));
		if (sysopt.maskref == NULL) {
			CDB_SHOW(("renamex: low memory\n"));
        	        return RNM_ERR_LOWMEM;
		}
		for (sysopt.mr_len = 0; (c < argc) && strcmp(argv[c], "-"); c++) {
			sysopt.maskref[sysopt.mr_len++] = argv[c];
		}
		c++;	/* skip the '-' */
	}
	while (c < argc) {
		if (infile) {
			rename_enfile(&sysopt, argv[c]);
		} else {
			rename_entry(&sysopt, argv[c]);
		}
		c++;
	}

	CDB_SHOW(("Total:%d  Renamed:%d  Failed:%d  No-change:%d  "
			"Existed:%d\n", sysopt.st_process, sysopt.st_success, 
			sysopt.st_failed, sysopt.st_same, sysopt.st_skip));
	rename_free_all(0);
	return 0;
}

static int rename_free_all(int sig)
{
	(void) sig;

	slog_csoup_close();

	if (sysopt.action == RNM_ACT_REGEX) {
		regfree(sysopt.preg);
	}
	if (sysopt.patbuf) {
		sysopt.patbuf = smm_free(sysopt.patbuf);
	}
	if (sysopt.maskbuf) {
		sysopt.maskbuf = smm_free(sysopt.maskbuf);
	}
	if (sysopt.maskref) {
		sysopt.maskref = smm_free(sysopt.maskref);
	}
	if (sysopt.rtpath) {
		smm_cwd_pop(sysopt.rtpath);
	}
#ifdef	CFG_GUI_ON
	mmgui_close(sysopt.gui);
#endif
	return 0;
}

/* the pattern accept the vi style, like
 *   -s/pattern/destination/g
 * or colon because MinGW would auto expand the path like
 *   -s/pattern/destination/g
 * in MinGW it will turned into something like
 *   -s/C:/MinGW/msys/1.0/pattern/destination/g
 * so instead we can use
 *   -s:pattern:destination:g
 */
static int cli_set_pattern(RNOPT *opt, char *optarg)
{
	char	*idx[4], *p;

	/* skip the first separater */
	if ((*optarg == '/') || (*optarg == ':')) {
		optarg++;
	}

	opt->patbuf = csc_strcpy_alloc(optarg, 0);
	csc_fixtoken(opt->patbuf, idx, 4, "/:");
    	opt->pattern = idx[0];
	opt->substit = idx[1];

	if (!opt->pattern || !opt->substit) {
		opt->patbuf = smm_free(opt->patbuf);
		return RNM_ERR_PARAM;
	}

	opt->pa_len = strlen(opt->pattern);
	opt->su_len = strlen(opt->substit);
	opt->action = RNM_ACT_FORWARD;
	opt->regflag = 0;
	opt->rpnum = 1;		/* default replace once */
    	for (p = idx[2]; p && *p; p++)  {
		switch (*p)  {
		case 'g':
		case 'G':
			opt->rpnum = 0;	/* 0 = unlimit */
			break;
		case 'b':
		case 'B':
			opt->action = RNM_ACT_BACKWARD;
			break;
		case 'e':
		case 'E':
	    		opt->action = RNM_ACT_EXTENSION;
	    		break;
		case 'i':
		case 'I':
			opt->regflag |= REG_ICASE;
			opt->compare = strncasecmp;
			break;
		case 'r':
		case 'R':
			opt->action = RNM_ACT_REGEX;
			break;
		case 'x':
		case 'X':
			opt->regflag |= REG_EXTENDED;
	    		opt->action = RNM_ACT_REGEX;
			break;
		default:
			if (isdigit(*p)) {
				opt->rpnum = (int) strtol(p, &p, 10);
				p--;
			}
			break;
		}
	}
	return RNM_ERR_NONE;
}

/* the mask accept the vi style, like
 *   -m/pattern1/pattern2
 * or colon because MinGW would auto expand the path like
 *   -m/pattern1/pattern2
 * in MinGW it will turned into something like
 *   -m/C:/MinGW/msys/1.0/pattern1/pattern2
 * so instead we can use
 *   -m:pattern1:pattern2
 */
static int cli_set_mask(RNOPT *opt, char *optarg)
{
	char	*idx[4];

	/* skip the first separater */
	if ((*optarg == '/') || (*optarg == ':')) {
		optarg++;
	}

	opt->maskbuf = csc_strcpy_alloc(optarg, 0);
	csc_fixtoken(opt->maskbuf, idx, 4, "/:");
    	opt->mskpat1 = idx[0];
	opt->mskpat2 = idx[1];

	if (!opt->mskpat1 || !opt->mskpat1) {
		opt->maskbuf = smm_free(opt->maskbuf);
		return RNM_ERR_PARAM;
	}
	return RNM_ERR_NONE;
}

static int rename_debug_trans_module(int cw, char *buf, int blen)
{
	if (cw & RENAME_MOD_CORE) {
		csc_strlcat(buf, "[RENAME]", blen);
	}
	if (cw & RENAME_MOD_GUI) {
		csc_strlcat(buf, "[GUI]", blen);
	}
	return SMM_ERR_NULL;
}

static int debug_main(RNOPT *opt, int argc, char **argv)
{
	FILE	*fp;
	char	*cmd;
	int	i;

	cmd = argv[0];
	if (**argv == '-') {
		cmd = &argv[0][8];
	}

	//printf("%s %d %s\n", cmd, argc, argv[0]);
	/* debug functions doesn't need parameters */
	if (!strcmp(cmd, "option")) {
		rename_option_dump(opt);
	} else if (!strcmp(cmd, "debug")) {
		CDB_SHOW(("Internal: SHOW\n"));
		CDB_ERROR(("Internal: ERROR\n"));
		CDB_WARN(("Internal: Warning\n"));
		CDB_INFO(("Internal: INFO\n"));
		CDB_DEBUG(("Internal: DEBUG\n"));
		CDB_PROG(("Internal: PROG\n"));
		CDB_MODL(("Internal: MODule\n"));
		CDB_FUNC(("Internal: function\n"));
	} else if (!strcmp(cmd, "create") && (argc > 1)) {
		fp = smm_fopen(argv[1], "w");
		if (fp == NULL) {
			printf("fopen: %s\n", argv[1]);
		} else {
			fclose(fp);
		}
	} else if (!strcmp(cmd, "verify") && (argc > 1)) {
		fp = smm_fopen(argv[1], "r");
		if (fp == NULL) {
			printf("fopen: not found\n");
		} else {
			fclose(fp);
			return 1;
		}
	} else if (!strcmp(cmd, "rawname")) {
		for (i = 0; i < argc; i++) {
			csc_memdump(argv[i], strlen(argv[i]), 
					16, CSC_MEMDUMP_NO_ADDR);
		}
	} else if (rename_open_buffer(opt, cmd) == RNM_ERR_NONE) {
		puts(opt->buffer);
	}
	return 0;
}

/*
static char *strcpy_stop(char *dest, char **sour, int stop)
{
	while (**sour != 0) {
		if (**sour == '\\') {
			(*sour)++; *dest++ = **sour;
			if (**sour == 0) {
				return dest;
			}
		} else if (**sour == stop) {
			break;
		} else {
			*dest++ = **sour;
		}
		(*sour)++;
	}
	*dest++ = 0;
	return dest;
}

static char *strcpy_char(char *dest, char **sour, int c)
{
	while (**sour == c) {
		*dest++ = **sour;
		(*sour)++;
	}
	*dest++ = 0;
	return dest;
}
*/

