
IUPLIBP	= $(shell echo ./iup/lib/*)

ifeq  ($(MSYSTEM),MINGW32)
all: libcsoup libregex libiup install

clean: libregex_clean
	make -C ./iup clean
	make -C ./libcsoup clean
else
  ifdef	USE_GTK2
all: libcsoup libiup-gtk2
  else
all: libcsoup libiup
  endif

clean:
	make -C ./iup clean
	make -C ./libcsoup clean
endif


libcsoup:
	if [ ! -f ./libcsoup/libcsoup.a ]; then \
		make -C ./libcsoup all; \
	fi

libiup:
	if [ ! -f $(IUPLIBP)/libiup.a ]; then \
		make -C ./iup do_all; \
	fi

libiup_gtk2:
	if [ ! -f $(IUPLIBP)/libiup.a ]; then \
		USE_GTK2=1 make -C ./iup do_all; \
	fi

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
