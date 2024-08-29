
IUPLIBP	= $(shell echo ./iup-porting/iup/lib/*)
INSTALL	= ../libmingw

ifdef	USE_GTK2
IUPCFG	= USE_GTK2=$(USE_GTK2)
else
IUPCFG	=
endif

.PHONY: all libcsoup libiup libregex libregex_clean clean \
	mingw_all mingw_clean mingw_install

ifeq	($(MSYSTEM),MINGW32)
all:
else
all: libcsoup libiup
endif

libcsoup:
	@echo Check Libcsoup presence
	if [ ! -d libcsoup ]; then git clone https://github.com/xuminic/libcsoup.git; fi
	if [ -d libcsoup ]; then make -C ./libcsoup all; fi

libiup:
	@echo Check IUP presence
	if [ ! -d iup-porting ]; then git clone https://github.com/xuminic/iup-porting.git; fi
	if [ -d iup-porting ]; then $(IUPCFG) make -C ./iup-porting; fi

libregex:
	if [ ! -f ./regex-20090805/Makefile ]; then \
		(cd ./regex-20090805 && ./configure --disable-shared && cd ..); \
	fi
	if [ ! -f ./regex-20090805/.libs/libregex.a ]; then \
		make -C ./regex-20090805 all; \
	fi

libregex_clean:
	if [ -f ./regex-20090805/Makefile ]; then \
		make -C ./regex-20090805 distclean; \
	fi

clean:
	if [ -d libcsoup ]; then make -C ./libcsoup clean; fi
	if [ -d iup-porting ]; then make -C ./iup-porting clean; fi

mingw_all: libcsoup libiup libregex

mingw_clean: clean libregex_clean

mingw_install:
	cp -f  ./libcsoup/libcsoup.a $(INSTALL)/lib
	cp -f  ./libcsoup/*.h $(INSTALL)/include
	cp -af ./iup/include/* $(INSTALL)/include/iup
	cp -f  ./iup/lib/mingw4/*.a $(INSTALL)/lib
	cp -f  ./regex-20090805/lib/regex.h $(INSTALL)/include
	cp -f  ./regex-20090805/.libs/libregex.a $(INSTALL)/lib
							        


