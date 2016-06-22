
/*  csc_cmp_file_extargs.c

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
#include <stdarg.h>

#include "libcsoup.h"

/* it must end by a NULL, otherwise it goes nasty. 
 * for example:
 * csc_cmp_file_extargs("my_file_name.ext", ".c", ".h", ".cc", NULL); */
int csc_cmp_file_extargs(char *fname, char *ext, ...)
{
	va_list	ap;
	char	*s;
	int	n;

	va_start(ap, ext);
	n = 1;
	s = va_arg(ap, char *);
	while (s) {
		if (!csc_cmp_file_extname(fname, s)) {
			va_end(ap);
			return 0;	/* return succeed */
		}
		s = va_arg(ap, char *);
		n++;
	}
	va_end(ap);
	return n;
}


