#ifndef _DISK_MAN_H
#define _DISK_MAN_H

#include <stdbool.h>
#include "structs.h"

struct doubly_list *get_dirs_content(const char *);
int rm_entry(const struct fdata *);
off_t get_total_disk_usage(const struct doubly_list *);
struct size_format get_proper_size_format(off_t);
int correct_dir_st_size(struct doubly_list *);
bool is_dot_entry(const char *);

#endif
