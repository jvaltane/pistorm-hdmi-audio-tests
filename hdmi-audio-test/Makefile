CC=m68k-amigaos-gcc
CFLAGS=-noixemul -I. -Iinclude -m68040 -Os
LDFLAGS=-noixemul -m68040
LIBS=-lamiga

all: test

test: test.o
	$(CC) $(LDFLAGS) test.o -o test $(LIBS)

test.o: test.c
	$(CC) $(CFLAGS) -c test.c

clean:
	rm -rf test test.o
