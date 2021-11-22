#ifndef _DISK_MAN_H
#define _DISK_MAN_H

#include "structs.h"

struct doubly_list *get_dirs_content(const char *);
int rm_entry(struct fdata *const);

#endif
