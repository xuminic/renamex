
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
SYSFLAG	= -DUNICODE -D_UNICODE -D_WIN32_IE=0x0500 -DWINVER=0x500
SYSLIB	= -lwsock32 -lwldap32 -lregex

# Options: -mwindows, -mconsole -mwindows, -Wl,--subsystem,windows
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

ifeq	($(SYSGUI),CFG_GUI_ON)
  # Reload the previous GTK setting from IUP, if exists.
  # so we don't need prefix USE_GTK2 every time to build renamex
  GTKVER := $(shell cat ./external/iup/VERSION.gtk 2> /dev/null)
  ifeq	($(GTKVER),2)
    USE_GTK2 = 1
  endif
  ifdef	USE_GTK2
    GTKINC := $(shell pkg-config gtk+-2.0 --cflags)
    GTKLIB := $(shell pkg-config gtk+-2.0 --libs)
    IUPCFG = USE_GTK2=$(USE_GTK2)
  else	# auto-detect
    GTKINC := $(shell pkg-config gtk+-3.0 --cflags 2> /dev/null)
    GTKLIB := $(shell pkg-config gtk+-3.0 --libs 2> /dev/null)
    ifeq  ($(GTKINC), )
      GTKINC := $(shell pkg-config gtk+-2.0 --cflags)
      GTKLIB := $(shell pkg-config gtk+-2.0 --libs)
      IUPCFG = USE_GTK2=1	# No GTK3 installed
    endif
  endif
  SYSINC += -I./external/iup/include $(GTKINC)
  SYSLIB += $(GTKLIB) -lX11
  SYSLDD += -L`echo ./external/iup/lib/*`
endif
endif

PREFIX  = /usr/local
BINDIR  = $(PREFIX)/bin
MANDIR  = $(PREFIX)/share/man/man1
APPDIR  = $(PREFIX)/share/applications
P_ICON	= /usr/share/icons/hicolor
M_ICON	= apps/rename-extension.png

DEBUG	= -g -O0 -DDEBUG
#DEBUG	= -O3
DEFINES = -D$(SYSGUI)
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
	  renamex-*.lsm renamex.1 renamex.pdf

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
.PHONY: all clean cleanobj extlib extclean release installer_win \
	release-win showdll install uninstall

all: $(TARGET)
	@echo GTK=$(GTKVER) $(IUPCFG)

$(TARGET): $(OBJS)
	$(IUPCFG) make -C external all
	$(CC) $(CFLAGS) $(SYSLDD) -o $@ $(OBJS) $(LIBS)

rename_icon.o: rename_icon.rc
	windres $< -o $@

renamex.pdf: renamex.1
	man -l -Tps $< | ps2pdf - $@

cleanobj:
	$(RM) -r $(OBJS)

clean: cleanobj
	$(RM) $(TARGET)

extlib:
	$(IUPCFG) make -C external all

extclean:
	$(IUPCFG) make -C external clean

	
ifeq	($(SYSTOOL),unix)
release: extclean
else
release: extclean installer_win
endif
	-if [ -d $(RELDIR) ]; then $(RM) -r $(RELDIR); fi
	-mkdir $(RELDIR)
	-$(CP) $(COMMONS) $(RELDIR)
	-$(CP) *.c *.h *.rc *.nsi Makefile $(RELDIR)
	-$(CP) -a libmingw $(RELDIR)
	-$(CP) -a external $(RELDIR)
	#-7z a -tzip $(RELDIR).zip $(RELDIR)
	-tar czf $(RELDIR).tar.gz $(RELDIR)
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

install:
	install -s renamex $(BINDIR)
	cp -f renamex.1 $(MANDIR)
	cp -f renamex.desktop $(APPDIR)
	cp -f ./external/icons/rename256.png $(P_ICON)/256x256/$(M_ICON)
	cp -f ./external/icons/rename128.png $(P_ICON)/128x128/$(M_ICON)
	convert -resize 48x48 ./external/icons/rename256.png $(P_ICON)/48x48/$(M_ICON)
	convert -resize 32x32 ./external/icons/rename256.png $(P_ICON)/32x32/$(M_ICON)
	convert -resize 24x24 ./external/icons/rename256.png $(P_ICON)/24x24/$(M_ICON)
	convert -resize 22x22 ./external/icons/rename256.png $(P_ICON)/22x22/$(M_ICON)
	convert -resize 16x16 ./external/icons/rename256.png $(P_ICON)/16x16/$(M_ICON)
	gtk-update-icon-cache -f $(P_ICON)

uninstall:
	rm -f $(BINDIR)/renamex 
	rm -f $(MANDIR)/renamex.1
	rm -f $(APPDIR)/renamex.desktop
	rm -f $(P_ICON)/256x256/$(M_ICON)
	rm -f $(P_ICON)/128x128/$(M_ICON)
	rm -f $(P_ICON)/48x48/$(M_ICON)
	rm -f $(P_ICON)/32x32/$(M_ICON)
	rm -f $(P_ICON)/24x24/$(M_ICON)
	rm -f $(P_ICON)/22x22/$(M_ICON)
	rm -f $(P_ICON)/16x16/$(M_ICON)

