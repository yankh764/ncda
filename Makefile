CC = gcc
CFLAGS = -march=native -O2 -pipe -fstack-protector-strong -Wall -Wextra \
		 -Wundef -Wformat=2 -Wstrict-overflow=5 -Winline -I$(INCLUDE)
LDFLAGS = -ltinfo

DST_DIR = /usr/local/bin

BIN ?= ncda
INCLUDE ?= include/
OBJDIR ?= build
SRCDIR ?= src

.PHONY: test

test: 
	$(CC) -ggdb $(CFLAGS) $(SRCDIR)/* test.c $(LDFLAGS)
