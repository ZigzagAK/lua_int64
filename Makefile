.PHONY: all

CFLAGS ?= -I/usr/local/include

all: E
	gcc -g -Wall -fPIC --shared -o int64.so int64.c $(CFLAGS)

E:
	gcc -E -o int64.E int64.c $(CFLAGS)