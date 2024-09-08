
ifdef	USE_GTK2
IUPCFG	= USE_GTK2=$(USE_GTK2)
else
IUPCFG	=
endif

.PHONY: all libcsoup libcsoup_clean libiup libiup_clean libregex libregex_clean clean help

ifeq ($(MSYSTEM),MINGW32)
all: libcsoup libiup libregex

clean: libcsoup_clean libiup_clean libregex_clean
else
all: libcsoup libiup

clean: libcsoup_clean libiup_clean
endif


libcsoup:
	@echo Check Libcsoup presence
	if [ ! -d libcsoup ]; then git clone https://github.com/xuminic/libcsoup.git; fi
	if [ -d libcsoup ]; then make -C ./libcsoup all; fi

libcsoup_clean:
	if [ -d libcsoup ]; then make -C ./libcsoup clean; fi

libiup:
	@echo Check IUP presence
	if [ ! -d iup-porting ]; then git clone https://github.com/xuminic/iup-porting.git; fi
	if [ -d iup-porting ]; then $(IUPCFG) make -C ./iup-porting; fi

libiup_clean:
	if [ -d iup-porting ]; then make -C ./iup-porting clean; fi

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
	rm -rf ./regex-20090805/autom4te.cache 

help:
	@echo "Useful targets"
	@echo " all               build all required libraries"
	@echo " clean             clean all library files and object files previously built"
	@echo " libcsoup          only build the libcsoup"
	@echo " libcsoup_clean    only clean everything of the libcsoup"
	@echo " libiup            only build the libiup"
	@echo " libiup_clean      only clean everything of the libiup"
	@echo " libregex          only build the libregex"
	@echo " libregex_clean    only clean everything of the libregex_clean"


