
/*  smm_cwd_alloc.c - get current working directory in a buffer

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

#include "libcsoup.h"

#ifdef	CFG_WIN32_API
char *smm_cwd_alloc(int extra)
{
	TCHAR	*wpath;
	int	len;

	if ((len = GetCurrentDirectory(0, NULL)) == 0) {
		smm_errno_update(SMM_ERR_GETCWD);
		return NULL;	/* system call failed */
	}
	len++;	/* expend it for the null char */

	/* first 3*len buffer is reserved for the multibyte convert and
	 * the last len size is for the getcwd call */
	if ((wpath = smm_alloc(sizeof(TCHAR) * len * 4 + extra)) == NULL) {
		smm_errno_update(SMM_ERR_LOWMEM);
		return NULL;	/* low memory */
	}
	if (GetCurrentDirectory(len, wpath + len * 3) == 0) {
		smm_errno_update(SMM_ERR_GETCWD);
		return NULL;	/* system call failed */
	}
	WideCharToMultiByte(smm_codepage(), 0, wpath + len * 3, -1, 
			(char*) wpath, sizeof(TCHAR) * len * 3, NULL, NULL);
	return (char*) wpath;
}
#endif

#ifdef	CFG_UNIX_API
#include <unistd.h>
#include <limits.h>
#include <errno.h>

char *smm_cwd_alloc(int extra)
{
	char	*path;
	int	i, len = PATH_MAX;

	for (i = 0; i < 8; i++) {
		if ((path = smm_alloc(len + extra)) == NULL) {
			smm_errno_update(SMM_ERR_LOWMEM);
			return NULL;
		}
		if (getcwd(path, len) != NULL) {
			return path;	/* successfully retrieve the CWD */
		}
		smm_free(path);
		if (errno != ERANGE) {	/* system call failed */
			smm_errno_update(SMM_ERR_GETCWD);
			return NULL;
		}

		len <<= 1;	/* enlarge the CWD buffer */
	}
	/* if it hits here, things must be weird */
	smm_errno_update(SMM_ERR_LOWMEM);
	return NULL;
}
#endif

