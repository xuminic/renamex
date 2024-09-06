
ifdef	USE_GTK2
IUPCFG	= USE_GTK2=$(USE_GTK2)
else
IUPCFG	=
endif

.PHONY: all libcsoup libiup libregex libregex_clean clean 

all: libcsoup libiup

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



