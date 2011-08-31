# Generated automatically from Makefile.in by configure.

DEBUG	= -g -DDEBUG

CC	= gcc
PREFIX	= /usr/local
BINDIR	= /usr/local/bin
MANDIR	= /usr/local/man/man1

DEFINES = -DHAVE_CONFIG_H
CFLAGS	= -Wall -O3 ${DEBUG} ${DEFINES}


OBJS	= rename.o fixtoken.o misc.o 

all: renamex

renamex : main.c fixtoken.c rename.c
	$(CC) $(CFLAGS) -o $@ $^
	cp $@ /usr/local/bin

static:	$(OBJS)
	$(CC) $(CFLAGS) -static -o $@ $^

.PHONY: clean clean-all install
clean:
	rm -f core renamex *.o 

clean-all: clean
	rm -f config.status config.cache config.h config.log Makefile

install:
	install -o root -g root -m 0755 -s rename $(BINDIR)
	install -o root -g root -m 0644 rename.1 $(MANDIR)
	
%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<


