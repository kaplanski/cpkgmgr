CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic
all: cpkgmgr

cpkgmgr: cpkgmgr.c
	 $(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f cpkgmgr
