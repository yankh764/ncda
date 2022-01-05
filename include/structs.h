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
	/* Equivalent to end of string */
	char eos;
};

struct entry_data {
	struct fdata *file_data;
	struct cdata *curses_data;
};

/* A doubly linked list for the entries found in a directory */
struct entries_dlist {
	struct entries_dlist *prev;
	struct entry_data *data;
	struct entries_dlist *next;
};

struct size_format {
	float val;
	char *unit;
};

void *alloc_entries_dlist(size_t, size_t);
void free_entries_dlist(struct entries_dlist *);

#endif
