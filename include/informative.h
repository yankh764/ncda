#ifndef _INFORMATIVE_H
#define _INFORMATIVE_H

#include <time.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

void *malloc_inf(size_t);
int lstat_inf(const char *, struct stat *);
DIR *opendir_inf(const char *);
int closedir_inf(DIR *);
struct dirent *readdir_inf(DIR *);
int unlink_inf(const char *);
int rmdir_inf(const char *);
int fclose_inf(FILE *fp);
FILE *fopen_inf(const char *path, const char *mode);
time_t time_inf(time_t *);

#endif
