#ifndef _STRUCTS_H
#define _STRUCTS_H

#include <sys/types.h>
#include <sys/stat.h>

/* File's data */
struct fdata {
        char *fname;
        char *fpath;
        struct stat *fstatus;
};

/* Ncurses data */
struct cdata {
	short cpair;
	int y;
	char eos; /* Equivalent to end of string */
};

struct entry_data {
	struct fdata *file_data;
	struct cdata *curses_data;
};

struct doubly_list {
	struct doubly_list *prev;
	struct entry_data *data;
	struct doubly_list *next;
};


void *alloc_doubly_list(size_t, size_t);
void free_doubly_list(struct doubly_list *);

#endif
