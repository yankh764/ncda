#ifndef _DISK_H
#define _DISK_H

#include "structs.h"

enum COLOR_PAIRS {
	BLUE_PAIR =  1,
	GREEN_PAIR = 2, 
	YELLOW_PAIR = 3,
	CYAN_PAIR = 4,
	MAGENTA_PAIR = 5,
	RED_PAIR = 6,
	DEFAULT_PAIR = 7
};

struct dtree *get_dir_tree(const char *);
int rm_entry(const struct fdata *);
void correct_dtree_st_size(struct dtree *);

#endif