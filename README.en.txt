
Rename 1.99
-----------

Rename is a tool to rename files. It can change, lowercase and uppercase
a batch of files, or modify their ownership. It's a small and quick tool 
written in C so it's quicker than most rename tools written in shell scripts.
Rename is powered by the extended regular expression for searching and 
substituting string patterns in the file names.


Features
--------

 * substitue strings in file's name
 * search and substitue strings in file's name by regular expression
 * uppercase or lowercase file's name
 * support batch renaming
 * recursively processing directories and subdirectories
 * support to read the filename list from a file
 * change ownership of files
 * safe mode: test before you go


Install
-------

Download rename distribution then unpack it with tar -zxf:

    make
    make install


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

