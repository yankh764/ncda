#ifndef _DISK_MAN_H
#define _DISK_MAN_H

#include "structs.h"

struct entries_dlist *get_dirs_content(const char *);
int rm_entry(const struct fdata *);
off_t get_total_disk_usage(const struct entries_dlist *);
struct size_format get_proper_size_format(off_t);
int correct_dir_st_size(struct entries_dlist *);

#endif
