
include Make.conf


ifeq	($(SYSTOOL),unix)
TARGET	= renamex
else
TARGET	= renamex.exe
endif

OBJS	= main.o  fixtoken.o	rename.o
MANPAGE	= renamex.1

all: libsmm $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lsmm

static:	$(OBJS)
	$(CC) $(CFLAGS) -static -o $@ $^

libsmm:
	make -C libsmm all

.PHONY: clean clean-all install libsmm
clean:
	$(RM) $(TARGET) $(OBJS)
	make -C libsmm clean

clean-all: clean
	$(RM) config.status config.cache config.h config.log Makefile

ifeq	($(SYSTOOL),mingw)
install:
	echo Not functional
else
install:
	install -o root -g root -m 0755 -s $(TARGET) $(BINDIR)
	install -o root -g root -m 0644 $(MANPAGE) $(MANDIR)
endif	

