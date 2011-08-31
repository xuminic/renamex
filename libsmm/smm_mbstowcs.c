
/*  smm_mbstowcs.c - convert the multibyte string to wide character string

    Copyright (C) 2011  "Andy Xuming" <xuming@users.sourceforge.net>

    This file is part of LIBSMM, System Masquerade Module library

    LIBSMM is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    LIBSMM is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>

#include "libsmm.h"

#ifdef  CFG_WIN32_API
#ifndef UNICODE
#define UNICODE
#endif
#include <windows.h>

void *smm_mbstowcs(char *mbs)
{
	TCHAR	*buf;
	int	len;

	len = MultiByteToWideChar(smm_sys_cp, 0, mbs, -1, NULL, 0);
	if ((buf = malloc((len + 1) * sizeof(TCHAR))) == NULL) {
		smm_errno_update(ERROR_NOT_ENOUGH_MEMORY);
		return NULL;
	}
	MultiByteToWideChar(smm_sys_cp, 0, mbs, -1, buf, len);
	return buf;
}
#endif

#ifdef  CFG_UNIX_API
#include <errno.h>

void *smm_mbstowcs(char *mbs)
{
	wchar_t	*buf;
	int	len;

	if ((len = mbstowcs(NULL, mbs, 0)) == 0) {
		return NULL;
	}
	if ((buf = malloc((len + 1) * sizeof(wchar_t))) == NULL) {
		smm_errno_update(ENOMEM);
		return NULL;
	}
	mbstowcs(buf, mbs, len + 1);
	return buf;
}
#endif
	

