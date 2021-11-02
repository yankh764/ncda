#ifndef _DISK_ANALYSIS_H
#define _DISK_ANALYSIS_H

#include <sys/stat.h>

struct fdata {
	unsigned int node_num;
        char *fname;
        char *fpath;
        struct stat *fstatus;
};

struct bin_tree {
        struct fdata *data;
        struct bin_tree *left;
        struct bin_tree *right;
};

#endif
