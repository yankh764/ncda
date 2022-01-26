#ifndef _DISK_MAN_H
#define _DISK_MAN_H

#include "structs.h"

struct dtree *get_dir_tree(const char *);
int rm_entry(const struct fdata *);
void correct_dtree_st_size(struct dtree *);

#endif
