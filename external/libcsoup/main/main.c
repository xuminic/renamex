
/*  main.c

    Copyright (C) 2013  "Andy Xuming" <xuming@users.sourceforge.net>

    This file is part of CSOUP, Chicken Soup library

    CSOUP is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CSOUP is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>

#include "libcsoup.h"
#include "csoup_internal.h"

#include "main_define.h"

int main(int argc, char **argv)
{
	SMMDBG	*dbgc;

	dbgc = slog_csoup_open(NULL, NULL);
	smm_init();

	if (argc < 2) {
		slogs(dbgc, SLOG_LVL_SHOWOFF, 
				"Usage: csoup COMMAND [args ...]\n");
		csc_cli_cmd_print(cmdlist, NULL);
	} else if (csc_cli_cmd_run(cmdlist, NULL, --argc, ++argv) == 
			CSC_CLI_UNCMD) {
		slogf(dbgc, SLOG_LVL_SHOWOFF, 
				"%s: command not found.\n", *argv);
	}
	slog_csoup_close();
	return -1;
}

