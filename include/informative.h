#ifndef _INFORMATIVE_H
#define _INFORMATIVE_H

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

void *malloc_inf(size_t);
int stat_inf(const char *, struct stat *);
DIR *opendir_inf(const char *);
int closedir_inf(DIR *);
struct dirent *readdir_inf(DIR *);
int unlink_inf(const char *);
int rmdir_inf(const char *);

#endif
