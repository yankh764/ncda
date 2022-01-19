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

/*
 * An data structure for the directories' tree.
 * It is some kind of a doubly linked list but with
 * an additional dtree structure as a child node.
 */
struct dtree {
	struct dtree *prev;
	struct entry_data *data;
	struct dtree *child;
	struct dtree *next;
};

struct size_format {
	float val;
	char *unit;
};

void *alloc_dtree(size_t, size_t);
void free_dtree(struct dtree *);

#endif
