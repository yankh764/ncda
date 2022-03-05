#ifndef _DISK_H
#define _DISK_H

#include "structs.h"

enum COLOR_PAIRS {
	DEFAULT_PAIR = 0,
	BLUE_PAIR =  1,
	GREEN_PAIR = 2, 
	YELLOW_PAIR = 3,
	CYAN_PAIR = 4,
	MAGENTA_PAIR = 5,
	RED_PAIR = 6
};

struct dtree *get_dir_tree(const char *);
int rm_entry(const struct fdata *);
void correct_dirs_fsize(struct dtree *);
off_t get_dtree_disk_usage(const struct dtree *);

#endif
