
/*  smm_init.c - initialize the SMM library

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

#include "libsmm.h"

int	smm_error_no;
int	smm_sys_cp;


int smm_init(void)
{
	smm_error_no = 0;
	smm_sys_cp   = smm_codepage();
	return 0;
}

int smm_errno(void)
{
	return - smm_error_no;
}


#ifdef  CFG_WIN32_API
#ifndef	UNICODE
#define UNICODE
#endif
#include <windows.h>

int smm_errno_update(int value)
{
	if (value) {
		smm_error_no = value;
	} else {
		smm_error_no = (int) GetLastError();
	}
	return smm_errno();
}
#endif

#ifdef	CFG_UNIX_API
#include <errno.h>

int smm_errno_update(int value)
{
	if (value) {
		smm_error_no = value;
	} else {
		smm_error_no = errno;
	}
	return smm_errno();
}
#endif

