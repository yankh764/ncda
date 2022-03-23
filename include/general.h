#ifndef _GENERAL_H 
#define _GENERAL_H

#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include "structs.h"

void free_and_null(void **);
bool is_dot_entry(const char *);
struct size_format get_proper_size_format(off_t);
char *get_mtime_str(time_t);
int efficient_strcmp(const char *, const char *);
size_t get_strsize(const char *);
char *extract_dir_path(const char *);

#endif
