
/*  main.c - command line mode entry

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pwd.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <regex.h>

#include "libcsoup.h"
#include "rename.h"

RNOPT	*sysopt = NULL;

static	struct	cliopt	clist[] = {
	{ 0, NULL, 0, "Usage: renamex [OPTIONS] filename ..." },
	{ 0, NULL, 0, "OPTIONS:" },
	{ 'f', "file",      1, "Load file names from the file" },
	{ 'l', "lowercase", 0, "Lowercase the file name" },
	{ 'u', "uppercase", 0, "Uppercase the file name" },
	{ 'p', "prefix",    1, "Add prefix to the file name" },
	{ 'x', "suffix",    1, "Add suffix to the file name" },
	{ 's', "search",    1, "search and replace in the file name" },
#ifdef	CFG_GUI_ON
	{ 'G', "gui",       0, "start the GUI mode" },
#endif
	{ 'R', "recursive", 0, "Operate on files and directories recursively" },
	{ 'v', "verbose",   0, "Display verbose information" },
	{ 't', "test",      0, "Test only mode. Nothing will be changed" },
	{ 'A', "always",    0, "Always overwrite the existing files" },
	{ 'N', "never",     0, "Never overwrite the existing files" },
	{   1, "help",      0, "Display the help message" },
	{   2, "version",   0, "Display the version message" },
	{ 0, NULL, 0, "\nSEARCH SETTING:\n\
  -s /PATTERN/STRING[/SW]  Replace the matching PATTERN by STRING.\n\
The SW could be:\n\
  [i] ignore case when searching\n\
  [b] backward searching and replacing\n\
  [s] change file's suffix name\n\
  [r] PATTERN is regular expression, see regex(7)\n\
  [e] PATTERN is extended regular expression, see regex(7)\n\
  [g] replace all occurrences in the filename\n\
  [1-9] replace specified occurrences in the filename\n" },
	{ 0, NULL, 0, NULL }
};


static	char	*version = "renamex " RENAME_VERSION
", Rename files by substituting the specified patterns.\n\
Copyright (C) 1998-2016 \"Andy Xuming\" <xuming@users.sourceforge.net>\n\
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\n\
This is free software: you are free to change and redistribute it.\n\
There is NO WARRANTY, to the extent permitted by law.\n";

static int rename_free_all(int sig);
static int rename_run_gui(RNOPT *opt, int argc, char **argv);
static int cli_set_pattern(RNOPT *opt, char *optarg);
static int cli_dump(RNOPT *opt);

int main(int argc, char **argv)
{
	void	*argp;
	int 	c, infile = 0, rc = RNM_ERR_NONE;

	smm_init();

	if ((sysopt = malloc(sizeof(RNOPT))) == NULL) {
		return RNM_ERR_LOWMEM;
	}
	memset(sysopt, 0, sizeof(RNOPT));

	if ((argp = csc_cli_getopt_open(clist)) == NULL) {
		return -1;
	}
	
#ifdef  CFG_GUI_ON
	if ((sysopt->gui = mmgui_open(sysopt, &argc, &argv)) == NULL) {
		return -2; /* the config file */
	}
#endif
	while ((c = csc_cli_getopt(argc, argv, argp)) > 0) {
		switch (c) {
		case 1:
			csc_cli_print(clist, NULL);
			rename_free_all(0);
			return RNM_ERR_HELP;
		case 2:
			puts(version);
			rename_free_all(0);
			return RNM_ERR_HELP;
		case 'f':
			infile = 1;
			break;
		case 'l':
			sysopt->oflags &= ~RNM_OFLAG_MASKCASE;
			sysopt->oflags |= RNM_OFLAG_LOWERCASE;
			break;
		case 'u':
			sysopt->oflags &= ~RNM_OFLAG_MASKCASE;
			sysopt->oflags |= RNM_OFLAG_UPPERCASE;
			break;
		case 'p':
			sysopt->oflags |= RNM_OFLAG_PREFIX;
			sysopt->prefix  = optarg;
			sysopt->pre_len = strlen(optarg);
			break;
		case 'x':
			sysopt->oflags |= RNM_OFLAG_SUFFIX;
			sysopt->suffix  = optarg;
			sysopt->suf_len = strlen(optarg);
			break;
#ifdef	CFG_GUI_ON
		case 'G':
			sysopt->cflags |= RNM_CFLAG_GUI;
			break;
#endif
		case 'R':
			sysopt->cflags |= RNM_CFLAG_RECUR;
			break;
		case 'v':
			sysopt->cflags |= RNM_CFLAG_VERBOSE;
			break;
		case 't':
			sysopt->cflags |= RNM_CFLAG_TEST | RNM_CFLAG_VERBOSE;
			break;
		case 'A':
			sysopt->cflags &= ~RNM_CFLAG_PROMPT_MASK;
			sysopt->cflags |= RNM_CFLAG_ALWAYS;
			break;
		case 'N':
			sysopt->cflags &= ~RNM_CFLAG_PROMPT_MASK;
			sysopt->cflags |= RNM_CFLAG_NEVER;
			break;
		case 's':
			rc = cli_set_pattern(sysopt, optarg);
			if (rc != RNM_ERR_NONE) {
				return rc;
			}
			break;
		}
	}

	sysopt->compare = strncmp;
	sysopt->rtpath  = smm_cwd_push();
	smm_signal_break(rename_free_all);

	if (sysopt->cflags & RNM_CFLAG_TEST) {
		cli_dump(sysopt);
	}

#ifdef	CFG_GUI_ON
	if (sysopt->cflags & RNM_CFLAG_GUI) {
		rc = rename_run_gui(sysopt, argc - optind, &argv[optind]);
		rename_free_all(0);
		return rc;
	}
#endif
	if (optind >= argc) {	/* no file name offered */
#ifdef	CFG_GUI_ON
		rc = rename_run_gui(sysopt, 0, NULL);
#else
		printf("%s: missing file operand\n", argv[0]);
		rc = RNM_ERR_PARAM;
#endif
		rename_free_all(0);
		return rc;
	}
	if (!sysopt->oflags && !sysopt->action) { /* no operation specified */
		/* if user request command like
		 *   renamex oldname newname
		 * the renamex will do the direct renaming for mv's sake.
		 * However renamex is NOT mv so it won't check the crosslink
		 * of devices or anything fancy */
		if (optind + 2 == argc) {
			rc = rename_executing(sysopt, argv[optind+1], argv[optind]);
		} else {
#ifdef	CFG_GUI_ON
			rc = rename_run_gui(sysopt, argc - optind, &argv[optind]);
#else
			printf("%s: missing rename operand\n", argv[0]);
			rc = RNM_ERR_PARAM;
#endif
		}
		rename_free_all(0);
		return rc;
	}

	for (c = optind; (c < argc) && (rc == RNM_ERR_NONE); c++) {
		if (infile) {
			rc = rename_enfile(sysopt, argv[c]);
		} else {
			rc = rename_entry(sysopt, argv[c]);
		}
	}

	printf("%d files renamed.\n", sysopt->rpcnt);
	rename_free_all(0);
	return rc;
}

static int rename_free_all(int sig)
{
	(void) sig;

	if (sysopt->action == RNM_ACT_REGEX) {
		regfree(sysopt->preg);
	}
	if (sysopt->buffer) {
		sysopt->buffer = smm_free(sysopt->buffer);
	}
	if (sysopt->patbuf) {
		sysopt->patbuf = smm_free(sysopt->patbuf);
	}
	if (sysopt->rtpath) {
		smm_cwd_pop(sysopt->rtpath);
	}
#ifdef	CFG_GUI_ON
	mmgui_close(sysopt->gui);
#endif
	free(sysopt);
	return 0;
}

static int rename_run_gui(RNOPT *opt, int argc, char **argv)
{
	int	i;

	for (i = 0; i < argc; i++) {
		puts(argv[i]);
	}
	mmgui_run(opt->gui);
	return 0;
}

static int cli_set_pattern(RNOPT *opt, char *optarg)
{
	char	*idx[4], *p;
	int	cflags = 0;

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
		case 's':
		case 'S':
	    		opt->action = RNM_ACT_SUFFIX;
	    		break;
		case 'i':
		case 'I':
			cflags |= REG_ICASE;
			opt->compare = strncasecmp;
			break;
		case 'r':
		case 'R':
			opt->action = RNM_ACT_REGEX;
			break;
		case 'e':
		case 'E':
			cflags |= REG_EXTENDED;
	    		opt->action = RNM_ACT_REGEX;
			break;
		default:
			if (isdigit(*p)) {
				opt->rpnum = *p - '0';
			}
			break;
		}
	}
	if ((opt->action == RNM_ACT_REGEX) &&
			regcomp(opt->preg, opt->pattern, cflags))  {
		printf("Wrong regular expression. [%s]\n", opt->pattern);
		return RNM_ERR_REGPAT;
	}
	return RNM_ERR_NONE;
}

static int cli_dump(RNOPT *opt)
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
	strcat(buf, "]");
	switch (opt->oflags & RNM_OFLAG_MASKCASE) {
	case RNM_OFLAG_LOWERCASE:
		strcat(buf, "[LOWCASE]");
		break;
	case RNM_OFLAG_UPPERCASE:
		strcat(buf, "[UPCASE]");
		break;
	}
	switch (opt->action) {
	case RNM_ACT_FORWARD:
		strcat(buf, "[FORWARD]");
		break;
	case RNM_ACT_BACKWARD:
		strcat(buf, "[BACKWARD]");
		break;
	case RNM_ACT_REGEX:
		strcat(buf, "[REGEX]");
		break;
	case RNM_ACT_SUFFIX:
		strcat(buf, "[SUFFIX]");
		break;
	}

	printf("Flags:          %s\n", buf);
	printf("Pattern:        %s (%d)\n", opt->pattern, opt->pa_len);
	printf("Substituter:    %s (%d)(x %d)\n", 
			opt->substit, opt->su_len, opt->rpnum);
	printf("Name Buffer:    %d\n", opt->room);
	printf("\n");
	return 0;
}

