
# Rename Express
Rename Express is a powerful tool designed for renaming files in bulk. 
It efficiently handles mass quantities of files, allowing you to batch change, 
lowercase, uppercase with ease. 
This lightweight tool, written in C, delivers faster performance than most 
renaming tools built with shell scripts.

Powered by regular expressions, Rename Express enables advanced search and 
substitution of string patterns across large file sets. 
It supports both MS Windows and Linux/Unix platforms, and includes a portable 
GUI frontend for user-friendly operation.

## Features
- directly search and replace substrings within filenames
- using regular expression to search and replace subtrings within filenames
- convert filenames to uppercase or lowercase
- fast renames large quantities of files
- recursively processing directories and subdirectories
- supports renaming by reading filenames from a file
- dry run mode: preview the results before making any changes
- support MS Windows and Linux/Unix
- support both CLI and GUI mode

## Install in Linux
Clone the source code from the repo, for example:
```
git clone https://github.com/xuminic/renamex.git
cd renamex
```
Or download the release package:
```
wget https://master.dl.sourceforge.net/project/rename/renamex-2.7.tar.bz2
tar jxf renamex-2.7.tar.bz2
cd renamex-2.7
```

Using the default configuration:
```
./configure
make
```
Depending on your system, if the libgtk_dev has been installed, it builds the
runtime with the GUI function. Otherwise it would build a CLI tool only.

To build the `rename express` with GUI, the following dependencies are required:

- libgtk-3-dev

or

- libgtk2.0-dev

The dependencies can be recognized by the output of `configure`, for example:
```
...
checking pkg-config is at least version 0.9.0... yes
checking for GTK... yes
configure: gtk+-3.0 found
checking for IupOpen in -liup... no
configure: The libiup will be built soon.
...
```
If both `libgtk-3-dev` and `libgtk2.0-dev` were installed, the `libgtk-3-dev` will 
be prioritized by defaul, unless manually overrided by:
```
./configure
USE_GTK2=1 make
```
which would link the libgtk2.0 instead.

If you wish to build a CLI only program even if the libgtk_dev had been installed, 
you may use the `--with-gui` option:
```
./configure --with-gui=no
make
```

Or make a command line only tool:

    SYSGUI=CFG_GUI_OFF make






## Examples
To lowercase all files' names recursively.
```
renamex -l -R *
```
Substitute all 'abc' substrings appeared in C  sources  files  with
'xyz', ignoring the case, then uppercase the whole file name.
```
renamex -u -s/abc/xyz/gi *.c
```
Find all files with the '.c' suffix in the current directory and change 
them to '.cpp' suffix. Print the verbose information.
```
renamex -v -s/.c/.cpp/e *
```

Find all files with the '.c' suffix under the current directory and change
them to '.cpp' suffix by the list file.
```
find . -name *.c > filename.lst
renamex -s/.c/.cpp/e -f filename.lst
```
Read names from the 'filename.lst', find the last occurrence of 'abc'  
and  replace it with '12345', ignoring the case.
```
renamex -s/abc/12345/bi *
```
The target substring starts with a capital letter, and ends with string 
'file'. There are 0 or any numbers of characters between the capital letter
and 'file'. The substring, if encountered in filenames, will be replaced
with 'nofile'.
```
renamex -s/^[A-Z].*file/nofile/r *
```
Similar to above, except it uses extended regular expression, such as
the '+' metacharacter, and replaces all matching strings with 'nofile'.
```
renamex -s/^[A-Z].+file/nofile/xg *
```
Test mode only. Simulate the rename process but no files would be 
actually changed.
```
renamex -t -s/^[A-Z].+file/nofile/xg *
```

