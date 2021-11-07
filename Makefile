CC = gcc
CFLAGS = -I$(INCLUDE) -march=native -O2 -pipe -fstack-protector-strong   \
		 -std=gnu90 -Wall -Wextra -Wformat=2 -Wstrict-overflow=5 -Winline 
LDFLAGS = -lncurses -ltinfo

DST_DIR = /usr/local/bin

BIN ?= ncda
INCLUDE ?= ./include/
OBJDIR ?= ./build
SRCDIR ?= ./src

SRCS := $(shell find $(SRCDIR) -iname "*.c")
OBJS := $(SRCS:%=$(OBJDIR)/%.o)


$(OBJDIR)/%.c.o: %.c 
	mkdir -p $(dir $@) 
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: test

test: $(OBJS)
	$(CC) $(OBJS) -o ./$@ $(LDFLAGS)
