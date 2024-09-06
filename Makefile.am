
# Those lines from 
# 387, "am--refresh: Makefile" (note: include)
# to 
# 429, "$(srcdir)/config.h.in:  $(am__configure_deps)"    (note: exclude)
# shall be manually removed after invoking 'automake' everytime.
# Otherwise the makefile scripts would check the irrelevant files
# like m4, configure.ac, etc and spit something like:
#   cd . && /bin/bash ./renamex/missing automake-1.16 --foreign
#   ./renamex/missing: line 81: automake-1.16: command not found


AUTOMAKE_OPTIONS = no-dependencies foreign
ACLOCAL_AMFLAGS = -I m4 --install
SUBDIRS = .

DEFS	+= $(SYSDEFS)

if	XAM_MSWIN
bin_PROGRAMS	= renamex 
renamex_SOURCES	= rename.c rename.h 
renamex_LDADD	= main_con.o rename_icon.o
if	XAM_GUI
BUILT_SOURCES 	= $(PREFLIB)
WIN_OBJS	= main_win.o rename.o mmgui.o rename_icon.o
endif
else
bin_PROGRAMS	= renamex
if	XAM_GUI
renamex_SOURCES	= main.c mmgui.c mmrc_icon_dialog.h mmrc_icon_error.h \
		  mmrc_icon_info.h mmrc_icon_warning.h rename.c rename.h
else
renamex_SOURCES	= main.c rename.c rename.h
endif
BUILT_SOURCES 	= $(PREFLIB)
endif

# This list of files for source release
EXTRA_DIST = config.h.in configure configure.ac config.guess config.sub \
	     aclocal.m4 install-sh Makefile.am Makefile.in missing \
	     COPYING README.md ChangeLog.txt  external \
	     autotest.sh main.c mmgui.c mmrc_icon_dialog.h mmrc_icon_error.h \
	     mmrc_icon_info.h mmrc_icon_warning.h rename.c rename.h \
	     rename.ico rename_icon.rc rename.nsi renamex.1 renamex.desktop \
	     renamex.lsm renamex.pdf

var:
	@echo CFLAGS = $(CFLAGS)
	@echo LDFLAGS = $(LDFLAGS)
	@echo LIBS = $(LIBS)
	@echo IUPCFG = $(IUPCFG)
	@echo COMPILE = $(COMPILE)
	@echo LINK = $(LINK)
	@echo PREFLIB = $(PREFLIB)
	@echo WIN_UNAME = $(WIN_UNAME)

help:
	@echo "Useful targets other than 'all'"
	@echo " clean             clean the object files of renamex only"
	@echo " distclean         clean the object files and configure files of renamex"
	@echo " extclean          clean the external libraries, i.e IUP and libcsoup"
	@echo " install           install the renamex to PREFIX/bin"
	@echo " gnome-install     also install the support files to GNOME desktop"
	@echo " gnome-uninstall   remove the support files of GNOME desktop"
	@echo " release           create the source tar ball with all dependencies"
	@echo " installer-win     create an installer in Windows (nsis required)"
	@echo " showdll           display the linked DLLs in Windows"

main_con.o : main.c
	$(COMPILE) -DCFG_GUI_OFF -c -o $@ $<

main_win.o : main.c
	$(COMPILE) -DCFG_GUI_ON -c -o $@ $<

if	XAM_MSWIN
distclean-local: extclean distclean-win
	rm -rf autom4te.cache
else
distclean-local: extclean
	rm -rf autom4te.cache
endif

release: extclean
	rm -rf "$(distdir)"
	mkdir "$(distdir)"
	cp -a $(EXTRA_DIST) "$(distdir)"
	rm -rf "$(distdir)/external/libcsoup/.git"
	rm -rf "$(distdir)/external/iup-porting/.git"
	tar cjf "$(distdir)-src.tar.bz2" "$(distdir)"
	rm -rf "$(distdir)"

gnome-install: install
	$(INSTALL_DATA) renamex.desktop /usr/share/applications
	@for res in 16 22 24 32 48 256; do \
	 $(INSTALL_DATA) -T ./external/icons/renamex"$$res".png \
	    /usr/share/icons/hicolor/"$$res"x"$$res"/apps/renamex.png; \
	done
	gtk-update-icon-cache -f /usr/share/icons/hicolor

gnome-uninstall: uninstall
	rm -f /usr/share/applications/renamex.desktop
	@for res in 16 22 24 32 48 256; do \
	  rm -f /usr/share/icons/hicolor/"$$res"x"$$res"/apps/renamex.png; \
	done
	gtk-update-icon-cache -f /usr/share/icons/hicolor

# supportive rule for generating files 
# since special tools are involved so they can be only internally used.

# since the pdf file is in the target list, the automake would try to rebuild
# it automatically when releasing. using an alter-form to avoid so.
renamex-pdf: renamex.1
	man -l -Tps $< | ps2pdf - renamex.pdf

gtkicons:
	@for res in 16 22 24 32 48 128 256; do \
	  convert -background transparent ./external/icons/rename256.png \
	    -resize "$$res"x"$$res" ./external/icons/renamex"$$res".png; \
	done

extlib:
	$(IUPCFG) make -C external all

extclean:
	$(IUPCFG) make -C external clean

if	XAM_MSWIN
all-local:
	if test -d "$(distdir)-$(WIN_UNAME)-bin"; then \
	  rm -rf "$(distdir)-$(WIN_UNAME)-bin"; \
	fi; \
	mkdir "$(distdir)-$(WIN_UNAME)-bin"; \
	cp -a renamex.1 renamex.pdf rename.ico "$(distdir)-$(WIN_UNAME)-bin"; \
	cp -a renamex*.exe autotest.sh "$(distdir)-$(WIN_UNAME)-bin"; \
	cp -a COPYING ChangeLog.txt README.md "$(distdir)-$(WIN_UNAME)-bin"

distclean-win:
	if test -f "renamex_win$(EXEEXT)"; then \
		rm -f "renamex_win$(EXEEXT)"; \
	fi; \
	if test -f "$(distdir)-$(WIN_UNAME)-setup.exe"; then \
		rm -f "$(distdir)-$(WIN_UNAME)-setup.exe"; \
	fi; \
	if test -d "$(distdir)-$(WIN_UNAME)-bin"; then \
		rm -rf "$(distdir)-$(WIN_UNAME)-bin"; \
	fi;

rename_icon.o: rename_icon.rc
	$(WINDRES) $< -o $@

renamex_win: $(WIN_OBJS)
	$(LINK) $^ $(WIN32GUI)

installer-win: all
	@echo "OutFile \"$(distdir)-$(WIN_UNAME)-setup.exe\"" > nsis_version.outfile
	$(NSIS) rename.nsi
	@$(RM) nsis_version.outfile

# show DLLs linked to renamex in MSWindows
showdll:
	@if [ -f renamex.exe ]; then \
		echo "[renamex.exe]:"; \
		objdump -p renamex.exe | grep 'DLL Name:'; \
	fi
	@if [ -f renamex_win.exe ]; then \
		echo "[renamex_win.exe]:"; \
		objdump -p renamex_win.exe | grep 'DLL Name:'; \
	fi
endif
