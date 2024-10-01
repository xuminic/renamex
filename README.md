
# Rename Express
Rename Express is a powerful tool designed for renaming files in bulk. 
It efficiently handles mass quantities of files, allowing you to batch change, 
lowercase, uppercase with ease. 
This lightweight tool, written in C, delivers faster performance than most 
renaming tools built with shell scripts.

Powered by regular expressions, Rename Express enables advanced search and 
substitution of string patterns across large file sets. 
It supports both MS Windows and Linux/Unix platforms, and includes a portable 
GUI front-end for user-friendly operation.

## Features
- directly search and replace substrings within filenames
- using regular expression to search and replace sub-strings within filenames
- convert filenames to uppercase or lowercase
- fast renames large quantities of files
- recursively processing directories and subdirectories
- supports renaming by reading filenames from a file
- dry run mode: preview the results before making any changes
- support MS Windows and Linux/Unix
- support both CLI and GUI mode

## Build in Linux
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
Depending on your system, if the `libgtk_dev` has been installed, it builds the
runtime with the GUI function. Otherwise it would build a CLI tool only.

To build the `rename express` with GUI, the following dependencies are required:

- `libgtk-3-dev`

or

- `libgtk2.0-dev`

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
be prioritized by default, unless manually overriden by:
```
./configure
USE_GTK2=1 make
```
which would link the `libgtk2.0` instead.

If you wish to build a CLI only program even if the `libgtk_dev` had been installed, 
you may use the `--with-gui` option:
```
./configure --with-gui=no
make
```

Or make a command line only tool:

    SYSGUI=CFG_GUI_OFF make


## Build in Windows
### Windows 10 and higher (todo)
1. Install MSYS2 (the build system and command line console)

2. Install tool chains

Install tool chains for MINGW32/MINGW64/UCRT:
```
pacman -S mingw-w64-i686-gcc mingw-w64-x86_64-gcc mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-i686-gdb mingw-w64-x86_64-gdb mingw-w64-ucrt-x86_64-gdb
pacman -S mingw32/mingw-w64-i686-nsis ucrt64/mingw-w64-ucrt-x86_64-nsis
```

3. Build the program

For example in MINGW32, using the default configuration:
```
git clone https://github.com/xuminic/renamex.git
cd renamex
./configure
make
```
When successful the build process would generate two executable files and
one install directory, for example:
```
renamex.exe
renamex_win.exe
renamex-2.11-win32-bin
```
where `renamex.exe` is a command line tool and `renamex_win.exe` is a Windows program.
All executable program and documents are grouped in the `renamex-2.11-win32-bin` folder.
Rename Express uses the native Windows API only so these executable programs are "green"
and portable, can be moved to anywhere to run.

4. Generate the Windows Installer (Optional)

Though the Rename Express is "green" and portal, it can still generate a traditional
Windows Installer by NSIS. Using this command:
```
make installer-win
```
it will generate a file like `renamex-2.11-win32-setup.exe`.

### Windows 7
1. Install MSYS2 (the build system and command line console)

MSYS2 does no longer support Windows 7. The
[2022-10-28](https://github.com/msys2/msys2-installer/releases/tag/2022-10-28)
is the last version for Windows 7. Install this version with required tools.

2. Lock the runtime

After installation of the `2022-10-28`, first thing is to lock the
`msys2-runtime` under `3.4.10-2`. Otherwise any upgrade or installation might introduce
the DLLs which Windows 7 can not link.

Run this command, but do NOT update anything, simply cancel the update.
```
pacman -Syu      # DO NOT DO UPDATE. Let it receive the metadata then quit; that's all
```
then upgrade the `msys2-runtime` to `3.4.10-2`:
```
pacman --noconfirm -S msys2-runtime-3.4 msys2-runtime-3.4-devel
```

3. OpenSSH

The OpenSSH 9.8 in MSYS2 2022-10-28 was very broken. 
It kept seeking a nonexistent file `msys_cryto_3.0.dll`. 
Extracting the `msys_cryto_3.0.dll` from higher version won't help. 
The `ssh` stop complaining the DLL yet display nothing instead.
The only workaround for me was rolling back to 9.1p1-1:
```
pacman -Rdd openssh
wget https://repo.msys2.org/msys/x86_64/openssh-9.1p1-1-x86_64.pkg.tar.zst
pacman -U openssh-9.1p1-1-x86_64.pkg.tar.zst
```

4. DO NOT UPDATE

Since fixed the runtime and openssh version, make it secure by being ignored in package
list. Edit the `/etc/pacman.conf`:
```
IgnorePkg = msys2-runtime openssh
```
Note that updating via `pacman -Syu` is doable now, but it breaks many packages.
So better avoid doing it.

5. Install tool chains

Install tool chains for MINGW32/MINGW64/UCRT:
```
pacman -S mingw-w64-i686-gcc mingw-w64-x86_64-gcc mingw-w64-ucrt-x86_64-gcc
pacman -S mingw-w64-i686-gdb mingw-w64-x86_64-gdb mingw-w64-ucrt-x86_64-gdb
pacman -S mingw32/mingw-w64-i686-nsis ucrt64/mingw-w64-ucrt-x86_64-nsis
```

6. Build the program

For example in MINGW32, using the default configuration:
```
git clone https://github.com/xuminic/renamex.git
cd renamex
./configure
make
```
When successful the build process would generate two executable files and
one install directory, for example:
```
renamex.exe
renamex_win.exe
renamex-2.11-win32-bin
```
where `renamex.exe` is a command line tool and `renamex_win.exe` is a Windows program.
All executable program and documents are grouped in the `renamex-2.11-win32-bin` folder.
Rename Express uses the native Windows API only so these executable programs are "green"
and portable, can be moved to anywhere to run.

7. Generate the Windows Installer (Optional)

Though the Rename Express is "green" and portal, it can still generate a traditional
Windows Installer by NSIS. Using this command:
```
make installer-win
```
it will generate a file like `renamex-2.11-win32-setup.exe`.


### Windows XP (32-bit)
1. Install MinGW (the build system and command line console)

Download the installer which still supports Windows XP from
[MinGW - Minimalist GNU for Windows Files](https://sourceforge.net/projects/mingw/files/)
and install everything within.

2. Install Git

Download Git from
[Git for Windows 2.10.0](https://github.com/git-for-windows/git/releases/tag/v2.10.0.windows.1)
and install it.

3. Install NSIS (Optional)

NSIS is used to generate the Windows Installer for distributing the software.
Download NSIS from
[NSIS: Nullsoft Scriptable Install System](https://sourceforge.net/projects/nsis).
Not sure what the highest version for Windows XP. At least my `2.46` worked fine.

4. Export the environment variable for Windows XP

To build the Rename Express in the MinGW console, the `MSYSTEM` need to be set to `MINGWXP`
```
export MSYSTEM=MINGWXP
```

5. Prepare the `config.h.in`

The `gawk 3.1.7` in MinGW seems not handling the CRLF line ends very well. 
The `configure` generates invalid `config.h`. 
The workaround is to pre-process the files with `dos2unix`.
```
git clone https://github.com/xuminic/renamex.git
cd renamex
dos2unix config.h.in
dos2unix external/regex-20090805/config.h.in
```

6. Build the program

Using the default configuration:
```
./configure
make
```
When successful the build process would generate two executable files and
one install directory, for example:
```
renamex.exe
renamex_win.exe
renamex-2.11-win32-bin
```
where `renamex.exe` is a command line tool and `renamex_win.exe` is a Windows program.
All executable program and documents are grouped in the `renamex-2.11-win32-bin` folder.
Rename Express uses the native Windows API only so these executable programs are "green"
and portable, can be moved to anywhere to run.

7. Generate the Windows Installer (Optional)

Though the Rename Express is "green" and portal, it can still generate a traditional
Windows Installer by NSIS. Using this command:
```
make installer-win
```
it will generate a file like `renamex-2.11-win32-setup.exe`.



### The Pre-built Installer

### Build from source code
Currently the Rename Express can be built with MinGW32 and MinGW64. 
UCRT64 and Cygwin are not supported yet.

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

By default the build process would generate two executable files and 
one install directory, for example:
```
renamex.exe
renamex_win.exe
renamex-2.11-win32-bin
```
where `renamex.exe` is a command line tool and `renamex_win.exe` is a Windows program.
Rename Express uses the native Windows API only so these executable programs are "green"
and portable, can be moved to anywhere to run.



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

