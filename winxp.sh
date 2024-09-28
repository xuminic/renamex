#!/bin/sh
# When linker in WinXP looking for ___ms_vsnprintf, it can be tricked by redefine the symbol
# to mingw one like ___mingw_vsnprintf. To do so, it can be formed like this:
#   LDFLAGS = -L./external/libcsoup -L./external/regex-20090805/.libs -L./external/iup-porting/lib \
#             -Wl,-u,___mingw_vsnprintf -Wl,--defsym,___ms_vsnprintf=___mingw_vsnprintf
#
# Sometimes regex-20090805 missed almost every RE_* macros. It can be tricked in
# external/regex-20090805/lib/regex.h, line 35. Changes:
#        || (!defined _POSIX_C_SOURCE && !defined _POSIX_SOURCE
# to
#        || (!defined _POSIX_SOURCE
#
# It can be made as external/regex-20090805/lib/regex_xp.h as backup plan. When in need:
#   cp -f external/regex-20090805/lib/regex_xp.h  external/regex-20090805/lib/regex.h
#
#
dos2unix ./config.h.in
dos2unix ./external/regex-20090805/config.h.in
