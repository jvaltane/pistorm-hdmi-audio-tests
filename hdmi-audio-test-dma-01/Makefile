CC=m68k-amigaos-gcc
CFLAGS=-noixemul -I. -Iinclude -m68040 -Os
LDFLAGS=-noixemul -m68040
LIBS=-lamiga

all: test

test: test.o dma.o mbox.o devicetree.o
	$(CC) $(LDFLAGS) test.o dma.o mbox.o devicetree.o -o test $(LIBS)

test.o: test.c
	$(CC) $(CFLAGS) -c test.c

dma.o: dma.c dma.h
	$(CC) $(CFLAGS) -c dma.c

mbox.o: mbox.c mbox.h
	$(CC) $(CFLAGS) -c mbox.c

devicetree.o: devicetree.c devicetree.h
	$(CC) $(CFLAGS) -c devicetree.c

clean:
	rm -rf test test.o dma.o mbox.o devicetree.o
