#ifndef _DISK_MAN_H
#define _DISK_MAN_H

#include "structs.h"

struct dtree *get_dir_tree(const char *);
int rm_entry(const struct fdata *);
off_t get_total_disk_usage(const struct dtree *);
int correct_dir_st_size(struct dtree *);

#endif
