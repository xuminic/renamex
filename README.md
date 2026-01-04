
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
- Pattern Matching mode can match differently named groups by shared identifier
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


### The Pre-built Packages and Installers
- `renamex-*.*-src-all.tar.bz2`: the source code tar ball with every dependencies included; can be built without Internet.
- `renamex-*.*-src.tar.bz2`: the source code tar ball with Github dependencies
- `renamex-*.*-win32-bin.zip`: the executable files package for 32-bit Windows, Windows 7 and above
- `renamex-*.*-win32-setup.exe`: the installer for 32-bit Windows, Windows 7 and above
- `renamex-*.*-win64-bin.zip`: the executable files package for 64-bit Windows, Windows 7 and above
- `renamex-*.*-win64-setup.exe`: the installer for 64-bit Windows, Windows 7 and above
- `renamex-*.*-winxp-bin.zip`: the executable files package for Windows XP, 32-bit
- `renamex-*.*-winxp-setup.exe`: the installer for Windows XP, 32-bit


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

Pattern matching mode, for two sets of files like
```
Season 01 - E01 This is file 1.mkv
Season 01 - E02 This is file 2.mkv
Season 01 - E03 This is file 3.mkv
```
and
```
S01E01 english.ass
S01E02 english.ass
S01E03 english.ass
```
This command
```
renamex -m/S01/english -s/mkv/ass/1 *.mkv - *.ass
```
can rename the `.ass` set to
```
Season 01 - E01 This is file 1.ass
Season 01 - E02 This is file 2.ass
Season 01 - E03 This is file 3.ass
```

Pattern matching mode could be tricky. 
Always using the `-t` option if not sure the output names are expected. 
For example
```
renamex -m/S01/english -ts/mkv/ass/1 *.mkv - *.ass
```

## 呪術廻戦
`呪術廻戦` 的视频文件和字幕文件看上去是这样的：
```
'[Judas] Jujutsu Kaisen - S01E04.mkv'  '呪術廻戦 第04話 「呪胎戴天」 (BD 1920x1080 x265 ALAC).sc.ass'
'[Judas] Jujutsu Kaisen - S01E05.mkv'  '呪術廻戦 第05話 「呪胎戴天－弐－」 (BD 1920x1080 x265 ALAC).sc.ass'
'[Judas] Jujutsu Kaisen - S01E06.mkv'  '呪術廻戦 第06話 「雨後」 (BD 1920x1080 x265 ALAC).sc.ass'
'[Judas] Jujutsu Kaisen - S01E07.mkv'  '呪術廻戦 第07話 「急襲」 (BD 1920x1080 x265 ALAC).sc.ass'
'[Judas] Jujutsu Kaisen - S01E08.mkv'  '呪術廻戦 第08話 「退屈」 (BD 1920x1080 x265 ALAC).sc.ass'
```
看上去字幕文件的命名规则更有意义，所以这次希望对照字幕文件改名，具体需求应该是这样的：
- 从 `[Judas] Jujutsu Kaisen - S01E04.mkv` 中抽取 `04`
- 根据 `04` ，从字幕文件组中找到
  `呪術廻戦 第04話 「呪胎戴天」 (BD 1920x1080 x265 ALAC).sc.ass`
- 把 `[Judas] Jujutsu Kaisen - S01E04.mkv` 改成 
  `呪術廻戦 第04話 「呪胎戴天」 (BD 1920x1080 x265 ALAC).mkv`
- 字幕文件名中的 `.sc` 很多余，把它去掉。

因此操作如下： 安全起见，我们先用测试命令：
```
$ renamex -m/S01E/.mkv -ts/sc.ass/mkv/1 *.ass - *.mkv
renaming: [Judas] Jujutsu Kaisen - S01E04.mkv
     -->  呪術廻戦 第04話 「呪胎戴天」 (BD 1920x1080 x265 ALAC).mkv : tested
renaming: [Judas] Jujutsu Kaisen - S01E05.mkv
     -->  呪術廻戦 第05話 「呪胎戴天－弐－」 (BD 1920x1080 x265 ALAC).mkv : tested
renaming: [Judas] Jujutsu Kaisen - S01E06.mkv
     -->  呪術廻戦 第06話 「雨後」 (BD 1920x1080 x265 ALAC).mkv : tested
renaming: [Judas] Jujutsu Kaisen - S01E07.mkv
     -->  呪術廻戦 第07話 「急襲」 (BD 1920x1080 x265 ALAC).mkv : tested
```
命令行分析如下
- `-m`:	 设置萃取范围
- `/S01E/.mkv`: 萃取范围从 `S01E` 开始，到 `.mkv` 结束
- `-t`: 测试效果，不改名
- `-s`: 指定搜索改名功能
- `/sc.ass/mkv`: 把文件名中的 `sc.ass` 改成 `mkv`
- `/1`: 只改一次
- `*.ass`: 参考文件名列表
- `-`: 参考文件名和备改文件名列表之间的隔离符
- `*.mkv` 备改文件名列表，这次需要改名的是 `.mkv` 文件

结果看上去是我们要的，因此去掉 `t` 参数
```
$ renamex -m/S01E/.mkv -s/sc.ass/mkv/1 *.ass - *.mkv
```
再删除字幕文件中的 `.sc`
```
$ renamex -s/.sc//1 *.ass
```
查看结果:
```
'呪術廻戦 第04話 「呪胎戴天」 (BD 1920x1080 x265 ALAC).ass'
'呪術廻戦 第04話 「呪胎戴天」 (BD 1920x1080 x265 ALAC).mkv'
'呪術廻戦 第05話 「呪胎戴天－弐－」 (BD 1920x1080 x265 ALAC).ass'
'呪術廻戦 第05話 「呪胎戴天－弐－」 (BD 1920x1080 x265 ALAC).mkv'
'呪術廻戦 第06話 「雨後」 (BD 1920x1080 x265 ALAC).ass'
'呪術廻戦 第06話 「雨後」 (BD 1920x1080 x265 ALAC).mkv'
'呪術廻戦 第07話 「急襲」 (BD 1920x1080 x265 ALAC).ass'
'呪術廻戦 第07話 「急襲」 (BD 1920x1080 x265 ALAC).mkv'
```
完全符合要求。
