
ifndef	SYSTOOL		# Options: unix, mingw
ifeq	($(MSYSTEM),MINGW32)
SYSTOOL = mingw
else
SYSTOOL	= unix
endif
endif

ifndef	SYSGUI		# Options: CFG_GUI_ON, CFG_GUI_OFF
SYSGUI	= CFG_GUI_ON
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
ifeq	($(SYSGUI),CFG_GUI_ON)
SYSLIB	+= -mwindows -lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 \
	   -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid -lcomctl32
else
SYSLIB	+= -mconsole
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
ifeq	($(SYSGUI),CFG_GUI_ON)
SYSINC	+= -I./external/iup/include `pkg-config gtk+-2.0 --cflags`
SYSLDD	+= -L$(shell echo ./external/iup/lib/*)
SYSLIB	+= `pkg-config gtk+-2.0 --libs` -lX11
endif
SYSFLAG =
endif

include	version.mk

PREFIX	= /usr/local
BINDIR	= /usr/local/bin
MANDIR	= /usr/local/man/man1

DEBUG	= -g -O0 -DDEBUG
#DEBUG	= -O3
DEFINES = -D$(SYSGUI) -DHAVE_CONFIG_H
CFLAGS  = -Wall -Wextra $(DEBUG) $(DEFINES) $(SYSINC) $(SYSFLAG)


PROJECT	= renamex

ifeq	($(SYSTOOL),unix)
TARGET	= $(PROJECT)
else 
ifeq	($(SYSGUI),CFG_GUI_OFF)
TARGET	= $(PROJECT).exe
else
TARGET	= $(PROJECT)_win.exe
endif
endif

RELDATE	= `date +%Y%m%d`
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

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
.PHONY: $(TARGET)

all: $(TARGET) version.mk

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

extlib:
	make -C ./external/iup do_all
	make -C ./external/libcsoup all

extinstall:
	cp -f  ./external/libcsoup/libcsoup.a ./libmingw/lib
	cp -f  ./external/libcsoup/libcsoup.h ./libmingw/include
	cp -af ./external/iup/include ./libmingw/include/iup
	cp -f  ./external/iup/lib/mingw4/*.a ./libmingw/lib

extclean:
	make -C ./external/iup clean
	make -C ./external/libcsoup clean

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
	-$(CP) *.c *.h *.pdf *.1 *.txt *.rc *.ico *.lsm *.png $(RELDIR)
	-$(CP) COPYING Makefile autotest.sh $(RELDIR)
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
	-$(CP) COPYING *.pdf *.1 *.txt *.ico autotest.sh $(RELWIN)
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


