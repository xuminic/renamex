
Rename 2.0
----------

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
 * change ownership of files
 * safe mode: test before you go


Install
-------

Download rename distribution then unpack it with tar -zxf:

    ./configure
    make
    make install


BUGS reporting
--------------

Please report bugs to <xuming@users.sourceforge.net>


Examples
--------

rename foo food
  Change file 'foo' to 'food', just like mv(1) does.

rename -lR *
  To lowcase all filenames, directories and filenames and directories
  under subdirectories.

rename -s/abc/xyz/gi *.c
  Substitute all 'abc' substrings appeared in C  sources  files  with
  'xyz', ignoring case.

rename -vs/.c/.cpp/s *.c
  Change  C sources suffix to C++ sources suffix, with verbose information.

rename -s/abc/12345/bi *
  Find the last occurrence of 'abc'  and  replace  it  with  '12345',
  ignoring case.

rename -o guest -R /home/custom
  change the owner of the file '/home/custom' to 'guest'. The 'guest' should
  be an effective user in the current system. If '/home/custom' is a directory,
  all files in this directory tree will hand to 'guest'.

rename -s/^[A-Z].*file/nofile/r *
  The target substring starts with a capital letter, and ends with string 
  'file'. There are 0 or any numbers of characters between the capital letter
  and 'file'. The substring, if encountered in filenames, will be replaced
  with 'nofile'.
  
rename -s/^[A-Z].+file/nofile/eg *
  Similar to last example, except it uses extended regular expression, such as
  the '+' metacharacter, and replaces all matching strings with 'nofile'.


