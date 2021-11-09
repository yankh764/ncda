#ifndef _STRUCTS_H
#define _STRUCTS_H

#include <sys/types.h>
#include <sys/stat.h>

struct fdata {
        char *fname;
        char *fpath;
        struct stat *fstatus;
};

/*
struct bin_tree {
        struct fdata *data;
        struct bin_tree *left;
        struct bin_tree *right;
};
*/

struct list {
	struct fdata *data;
	struct list *next;
};

void *alloc_stat();
void free_fdata(struct fdata *);
void *alloc_fdata(size_t, size_t);
/*
void *alloc_bin_tree();
void free_bin_tree(struct bin_tree *);
*/
void *alloc_list();
void free_list(struct list *);

#endif
