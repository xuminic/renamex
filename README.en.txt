
Rename Extension
----------------

Rename Extension is a tool to rename files. It can change, lowercase and 
uppercase a batch of files, or modify their ownership. It's a small tool 
written in C so it's quicker than most rename tools written in shell scripts.
Rename is powered by the extended regular expression for searching and 
substituting string patterns in the file names. It supports MS Windows and
Linux/Unix system with a portable GUI frontend.


Features
--------

 * substitue strings in file's name
 * search and substitue strings in file's name by regular expression
 * uppercase or lowercase file's name
 * support batch renaming
 * recursively processing directories and subdirectories
 * support to read the filename list from a file
 * safe mode: test before you go
 * support MS Windows and Linux/Unix system'
 * integrated a GUI frontend

Release Notes
-------------

Version 2.3:
 * Fixed the problem that Ctrl-A could not select-all in MS Windows.
 * Updated the build system to support GTK3

Version 2.2:
 * Fixed the bug that displaying ??? against UTF-8 Eastern Asia characters
   in MS Windows.
 * Updated the debug scheme so the tool works in MS Windows.
 
Version 2.1:
 * Upsourced to the recent version of IUP 3.19.1
 * Updated the icon and adapting the system fonts.

Version 2.0:
 * Programmed a GUI based on IUP for X-Window and MS Windows
 * Ported to Win32 API
 * Reviewed the original code and update to the new baseline


Install
-------

Download rename distribution then unpack it with tar -zxf, then make 
the default build which includes a GUI:

    make

Or make a command line only tool:

    SYSGUI=CFG_GUI_OFF make

In X Window with GTK2 and GTK3 installed, the rename extension tool will
pick GTK3 as default. It can be overrided by:

    USE_GTK2=1 make


BUGS reporting
--------------

Please report bugs to <xuming@users.sourceforge.net>


Examples
--------

renamex -l -R *
  To lowercase all files' names recursively.

renamex -u -s/abc/xyz/gi *.c
  Substitute all 'abc' substrings appeared in C  sources  files  with
  'xyz', ignoring the case, then uppercase the whole file name.

renamex -v -s/.c/.cpp/e *
  Find all files with the '.c' suffix in the current directory and change 
  them to '.cpp' suffix. Print the verbose information.

find . -name *.c > filename.lst
renamex -s/.c/.cpp/e -f filename.lst
  Find all files with the '.c' suffix under the current directory and change
  them to '.cpp' suffix by the list file.

renamex -s/abc/12345/bi *
  Read names from the 'filename.lst', find the last occurrence of 'abc'  
  and  replace it with '12345', ignoring the case.

renamex -s/^[A-Z].*file/nofile/r *
  The target substring starts with a capital letter, and ends with string 
  'file'. There are 0 or any numbers of characters between the capital letter
  and 'file'. The substring, if encountered in filenames, will be replaced
  with 'nofile'.
  
renamex -s/^[A-Z].+file/nofile/xg *
  Similar to above, except it uses extended regular expression, such as
  the '+' metacharacter, and replaces all matching strings with 'nofile'.

renamex -t -s/^[A-Z].+file/nofile/xg *
  Test mode only. Simulate the rename process but no files would be 
  actually changed.

