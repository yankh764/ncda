CC = gcc
CFLAGS = -I$(INCLUDE) -march=native -O2 -pipe -fstack-protector-strong   \
		 -std=gnu90 -Wall -Wextra -Wformat=2 -Wstrict-overflow=5 -Winline \
		 -Wundef -D_FILE_OFFSET_BITS=64
LDFLAGS = -lncurses -ltinfo

DST_DIR = /usr/local/bin

BIN ?= ncda
INCLUDE ?= ./include/
OBJDIR ?= ./build
SRCDIR ?= ./src

SRCS := $(shell find $(SRCDIR) -iname "*.c")
OBJS := $(SRCS:%=$(OBJDIR)/%.o)


#$(BIN): $(OBJS)
#	$(CC) $(OBJS) -o ./$@ $(LDFLAGS)
#
#$(OBJDIR)/%.c.o: %.c 
#	mkdir -p $(dir $@) 
#	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: test clean

test: 
	$(CC) $(CFLAGS) -ggdb $(SRCDIR)/* test.c $(LDFLAGS)

#clean:
#	$(RM) -r $(OBJDIR)
