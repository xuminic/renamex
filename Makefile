
ifndef	SYSTOOL		# Options: unix, mingw
  ifeq	($(MSYSTEM),MINGW32)
    SYSTOOL = mingw
  else
    SYSTOOL = unix
  endif
endif

ifndef	SYSGUI		# Options: CFG_GUI_OFF, CFG_GUI_ON
  SYSGUI = CFG_GUI_ON
endif

ifeq	($(SYSTOOL),mingw)
CC	= gcc -mms-bitfields
AR	= ar
CP	= cp
RM	= rm -f

SYSINC  = -I./libmingw/include -I./libmingw/include/iup
SYSLDD  = -L./libmingw/lib
SYSFLAG	= -DUNICODE -D_UNICODE -D_WIN32_IE=0x0500 -DWINVER=0x500 \
	  -DNONDLL #For linking static libgd
# Options: -mwindows, -mconsole -mwindows, -Wl,--subsystem,windows
#SYSLIB	= -ljpeg -lpng -lz -lwsock32 -lwldap32 -lregex
SYSLIB	= -lwsock32 -lwldap32 -lregex
ifeq	($(SYSGUI),CFG_GUI_OFF)
  SYSLIB += -mconsole
else
  SYSLIB += -mwindows -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 \
	     -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lcomctl32
endif
endif

# This setting is used for POSIX environment with the following libraries
# installed: GTK+, FreeType and libgd
ifeq	($(SYSTOOL),unix)
CC	= gcc
AR	= ar
CP	= cp
RM	= rm -f

SYSINC	= -I./external/libcsoup 
SYSLDD	= -L./external/libcsoup
SYSLIB	=
SYSFLAG =
OBJLIB	= ./external/libcsoup/libcsoup.a

ifeq	($(SYSGUI),CFG_GUI_ON)
  ifdef	USE_GTK2
    GTKINC := $(shell pkg-config gtk+-2.0 --cflags)
    GTKLIB := $(shell pkg-config gtk+-2.0 --libs)
  else
    GTKINC := $(shell pkg-config gtk+-3.0 --cflags)
    GTKLIB := $(shell pkg-config gtk+-3.0 --libs)
    ifeq  ($(GTKINC), )
      GTKINC := $(shell pkg-config gtk+-2.0 --cflags)
      GTKLIB := $(shell pkg-config gtk+-2.0 --libs)
    endif
  endif
  SYSINC += -I./external/iup/include $(GTKINC)
  SYSLIB += $(GTKLIB) -lX11
  SYSLDD += -L$(shell echo ./external/iup/lib/*)
  OBJLIB += $(shell echo ./external/iup/lib/*)/libiup.a
endif
endif

PREFIX	= /usr/local
BINDIR	= /usr/local/bin
MANDIR	= /usr/local/man/man1

DEBUG	= -g -O0 -DDEBUG
#DEBUG	= -O3
DEFINES = -D$(SYSGUI) -DHAVE_CONFIG_H
CFLAGS  = -Wall -Wextra $(DEBUG) $(DEFINES) $(SYSINC) $(SYSFLAG)


PROJECT	= renamex

ifeq	($(SYSTOOL),unix)
  TARGET = $(PROJECT)
else 
  ifeq	($(SYSGUI),CFG_GUI_OFF)
    TARGET = $(PROJECT).exe
  else
    TARGET = $(PROJECT)_win.exe
  endif
endif

RELDATE	:= $(shell date +%Y%m%)
RELVERS	:= $(shell grep RENAME_VERSION rename.h | cut -d\" -f 2)
RELDIR	= $(PROJECT)-$(RELVERS)
RELWIN	= $(RELDIR)-win32-bin

OBJS	= main.o rename.o
LIBS	= -lcsoup 
ifeq	($(SYSGUI),CFG_GUI_ON)
OBJS	+= mmgui.o
ifeq	($(SYSTOOL),mingw)
OBJS	+= rename_icon.o
endif
LIBS	+= -liup -liupimglib
endif
LIBS	+= $(SYSLIB)


COMMONS	= COPYING ChangeLog.txt README.en.txt autotest.sh rename.ico \
	  renamex-2.0.lsm renamex.1 renamex.pdf

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
.PHONY: $(TARGET)

all:
	@echo $(SYSINC) 
	@echo $(SYSLIB) $(SYSLDD)
	@echo $(RELDIR)
	@echo $(LIBS)


#all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(SYSLDD) -o $@ $(OBJS) $(LIBS)

rename_icon.o: rename_icon.rc
	windres $< -o $@

renamex.pdf: renamex.1
	man -l -Tps $< | ps2pdf - $@

cleanobj:
	$(RM) -r $(OBJS)

clean: cleanobj
	$(RM) $(TARGET)

regexlib:
	cd ./external/regex-20090805 && ./configure --disable-shared && \
		cd ../.. && make -C ./external/regex-20090805 all

regexclean:
	-if [ -f ./external/regex-20090805/Makefile ]; \
		then make -C ./external/regex-20090805 distclean; fi

ifeq	($(SYSTOOL),unix)
extlib:
else
extlib: regexlib
endif
	$(IUPCOND) make -C ./external/iup do_all
	make -C ./external/libcsoup all

ifeq	($(SYSTOOL),unix)
extclean:
else
extclean: regexclean
endif
	make -C ./external/iup clean
	make -C ./external/libcsoup clean

extinstall:
	cp -f  ./external/libcsoup/libcsoup.a ./libmingw/lib
	cp -f  ./external/libcsoup/libcsoup.h ./libmingw/include
	cp -f  ./external/libcsoup/libcsoup_debug.h ./libmingw/include
	cp -af ./external/iup/include/* ./libmingw/include/iup
	cp -f  ./external/iup/lib/mingw4/*.a ./libmingw/lib
	cp -f  ./external/regex-20090805/lib/regex.h ./libmingw/include
	cp -f  ./external/regex-20090805/.libs/libregex.a ./libmingw/lib
	
version.mk: rename.h 
	echo -n "RELVERS	= " > $@
	grep RENAME_VERSION $< | cut -d\" -f 2 >> $@
ifeq	($(SYSTOOL),unix)
	make extlib
endif

ifeq	($(SYSTOOL),unix)
release: extclean
else
release: extclean installer_win
endif
	-if [ -d $(RELDIR) ]; then $(RM) -r $(RELDIR); fi
	-mkdir $(RELDIR)
	-$(CP) $(COMMONS) $(RELDIR)
	-$(CP) *.c *.h *.rc *.nsi Makefile version.mk $(RELDIR)
	-$(CP) -a libmingw $(RELDIR)
	-$(CP) -a external $(RELDIR)
	-7z a -tzip $(RELDIR).zip $(RELDIR)
	-$(RM) -r $(RELDIR)

installer_win: release-win
	-echo "OutFile \"$(RELDIR)-setup.exe\"" > nsis_version.outfile
	makensis rename.nsi
	-$(RM) nsis_version.outfile

release-win: 
	-if [ -d $(RELWIN) ]; then $(RM) -r $(RELWIN); fi
	-mkdir $(RELWIN)
	-$(CP) $(COMMONS) $(RELWIN)
	SYSGUI=CFG_GUI_OFF make clean
	SYSGUI=CFG_GUI_OFF make
	-$(CP) $(PROJECT).exe $(RELWIN)
	SYSGUI=CFG_GUI_ON make clean
	SYSGUI=CFG_GUI_ON make
	-$(CP) $(PROJECT)_win.exe $(RELWIN)
	-7z a -tzip $(RELWIN).zip $(RELWIN)
	-$(RM) -r $(RELWIN)

showdll:
	@if [ -f $(PROJECT).exe ]; then \
		echo "[$(PROJECT).exe]:"; \
		objdump -p $(PROJECT).exe | grep 'DLL Name:'; \
	fi
	@if [ -f $(PROJECT)_win.exe ]; then \
		echo "[$(PROJECT)_win.exe]:"; \
		objdump -p $(PROJECT)_win.exe | grep 'DLL Name:'; \
	fi


P_ICON	= /usr/share/icons/hicolor
M_ICON	= apps/rename-extension.png

install:
	cp -f renamex /usr/bin
	cp -f renamex.1 /usr/share/man/man1
	cp -f renamex.desktop /usr/share/applications
	cp -f ./external/icons/rename256.png $(P_ICON)/256x256/$(M_ICON)
	cp -f ./external/icons/rename128.png $(P_ICON)/128x128/$(M_ICON)
	convert -resize 48x48 ./external/icons/rename256.png $(P_ICON)/48x48/$(M_ICON)
	convert -resize 32x32 ./external/icons/rename256.png $(P_ICON)/32x32/$(M_ICON)
	convert -resize 24x24 ./external/icons/rename256.png $(P_ICON)/24x24/$(M_ICON)
	convert -resize 22x22 ./external/icons/rename256.png $(P_ICON)/22x22/$(M_ICON)
	convert -resize 16x16 ./external/icons/rename256.png $(P_ICON)/16x16/$(M_ICON)
	rm $(P_ICON)/icon-theme.cache

uninstall:
	rm -f /usr/bin/renamex 
	rm -f /usr/share/man/man1/renamex.1
	rm -f /usr/share/applications/renamex.desktop
	rm -f $(P_ICON)/256x256/$(M_ICON)
	rm -f $(P_ICON)/128x128/$(M_ICON)
	rm -f $(P_ICON)/48x48/$(M_ICON)
	rm -f $(P_ICON)/32x32/$(M_ICON)
	rm -f $(P_ICON)/24x24/$(M_ICON)
	rm -f $(P_ICON)/22x22/$(M_ICON)
	rm -f $(P_ICON)/16x16/$(M_ICON)

