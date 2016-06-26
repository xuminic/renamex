
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

RENOP	*sysopt = NULL;



static	struct	cliopt	clist[] = {
	{ 0, NULL, 0, "Usage: renamex [OPTIONS] filename ..." },
	{ 0, NULL, 0, "OPTIONS:" },
	{ 'f', "file",      1, "Load file names from the file" },
	{ 'l', "lowercase", 0, "Lowercase the file name" },
	{ 'u', "uppercase", 0, "Uppercase the file name" },
	{ 'p', "prefix",    1, "Add prefix to the file name" },
	{ 'x', "suffix",    1, "Add suffix to the file name" },
	{ 's', "search",    1, "search and replace in the file name" },
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
static int cli_set_pattern(RENOP *opt, char *optarg);
static int cli_dump(RENOP *opt, char *filename);

int main(int argc, char **argv)
{
	void	*argp;
	int 	c, infile = 0, rc = RNM_ERR_NONE;

	smm_init();

	if ((sysopt = malloc(sizeof(RENOP))) == NULL) {
		return RNM_ERR_LOWMEM;
	}
	memset(sysopt, 0, sizeof(RENOP));

	if ((argp = csc_cli_getopt_open(clist)) == NULL) {
		return -1;
	}
	
#ifdef  CFG_GUI_ON
	if ((mopt.gui = mmgui_open(&mopt, &argc, &argv)) == NULL) {
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
			sysopt->prefix = optarg;
			break;
		case 'x':
			sysopt->oflags |= RNM_OFLAG_SUFFIX;
			sysopt->suffix = optarg;
			break;
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

	if ((optind >= argc) || (!sysopt->oflags && !sysopt->action)) {
		csc_cli_print(clist, NULL);
		rename_free_all(0);
		return RNM_ERR_HELP;
	}
	
	sysopt->compare = strncmp;
	sysopt->rtpath  = smm_cwd_push();
	smm_signal_break(rename_free_all);

	if (sysopt->cflags & RNM_CFLAG_TEST) {
		cli_dump(sysopt, argv[optind]);
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

#ifdef	CFG_REGEX
	if (sysopt->action == RNM_ACT_REGEX) {
		regfree(sysopt->preg);
	}
#endif
	if (sysopt->buffer) {
		sysopt->buffer = smm_free(sysopt->buffer);
	}
	if (sysopt->patbuf) {
		sysopt->patbuf = smm_free(sysopt->patbuf);
	}
	if (sysopt->rtpath) {
		smm_cwd_pop(sysopt->rtpath);
	}
	free(sysopt);
	return 0;
}

static int cli_set_pattern(RENOP *opt, char *optarg)
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
	opt->count = 1;		/* default replace once */
    	for (p = idx[2]; p && *p; p++)  {
		switch (*p)  {
		case 'g':
		case 'G':
			opt->count = 0;	/* 0 = unlimit */
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
#ifdef	CFG_REGEX
		case 'r':
		case 'R':
			opt->action = RNM_ACT_REGEX;
			break;
		case 'e':
		case 'E':
			cflags |= REG_EXTENDED;
	    		opt->action = RNM_ACT_REGEX;
			break;
#endif
		default:
			if (isdigit(*p)) {
				opt->count = *p - '0';
			}
			break;
		}
	}
#ifdef	CFG_REGEX
	if ((opt->action == RNM_ACT_REGEX) &&
			regcomp(opt->preg, opt->pattern, cflags))  {
		printf("Wrong regular expression. [%s]\n", opt->pattern);
		return RNM_ERR_REGPAT;
	}
#endif
	return RNM_ERR_NONE;
}

static int cli_dump(RENOP *opt, char *filename)
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

	printf("Source:         %s\n", filename);
	printf("Flags:          %s\n", buf);
	printf("Pattern:        %s (%d)\n", opt->pattern, opt->pa_len);
	printf("Substituter:    %s (%d)(x %d)\n", 
			opt->substit, opt->su_len, opt->count);
	printf("Name Buffer:    %d (%d)\n", opt->room, RNM_PATH_MAX);
	printf("\n");
	return 0;
}



