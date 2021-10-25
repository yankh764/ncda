CC = gcc
CFLAGS = -march=native -O2 -pipe -fstack-protector-strong -Wextra \
		 -Wall -Wundef -Wformat=2 -Wstrict-overflow=5 -I$(INCLUDE)
LDFLAGS = -ltinfo

DST_DIR = /usr/local/bin

BIN ?= ncda
INCLUDE ?= include/
OBJDIR ?= build
SRCDIR ?= src

