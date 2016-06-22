/*  csc_file_store.c - store the memory contents into file. 

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

long csc_file_store(char *path, int ovrd, char *src, long len)
{
	FILE	*fp;
	long	n, amnt;

	if (ovrd) {
		fp = fopen(path, "w");
	} else {
		fp = fopen(path, "a");
	}
	if (fp == NULL) {
		return SMM_ERR_OPEN;
	}

	amnt = 0;
	while (1) {
		n = fwrite(src + amnt, 1, len - amnt, fp);
		amnt += n;
		if (amnt == len) {
			break;
		}
	}
	fclose(fp);
	return amnt;
}


