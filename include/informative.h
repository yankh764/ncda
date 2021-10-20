#ifndef INFORMATIVE_H
#define INFORMATIVE_H

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

void *malloc_inf(size_t);
int stat_inf(const char *, struct stat *);
DIR *opendir_inf(const char *);
int closedir_inf(DIR *);

#endif
