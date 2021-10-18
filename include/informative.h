#ifndef INFORMATIVE_H
#define INFORMATIVE_H

#include <sys/stat.h>
#include <sys/types.h>

void *malloc_inf(size_t);
int stat_inf(const char *, struct stat *);

#endif
