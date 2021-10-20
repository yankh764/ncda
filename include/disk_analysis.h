#ifndef DISK_ANALYSIS_H
#define DISK_ANALYSIS_H

#include <sys/stat.h>

struct file_info {
        char *name;
        char *path;
        struct stat status;
}

struct bin_tree {
        void *info;
        struct bin_tree *left;
        struct bin_tree *right;
}

#endif
