/*
-----------------------------------------------------------
| License: GNU GPL-3.0                                    |
-----------------------------------------------------------
| This source file contains wrapper functions for some of |
| the GNU C library functions with informative error mes- |
| sages added to them in case of failures.                |
-----------------------------------------------------------
*/
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include "informative.h"

int ERROR = 0;


void *malloc_inf(size_t size)
{
        void *retval;

        if(!(retval = malloc(size))) {
		ERROR = errno;
                error(0, errno, "could not allocate memory");
	}
	return retval;
}

int lstat_inf(const char *path, struct stat *statbuf)
{
        int retval;

        if ((retval = lstat(path, statbuf))) {
		ERROR = errno;
                error(0, errno, "could not get entry's status '%s'", path);
	}
        return retval;
}

DIR *opendir_inf(const char *path)
{
        DIR *retval;

        if (!(retval = opendir(path))) {
		ERROR = errno;
                error(0, errno, "could not open directory '%s'", path);
	}
        return retval;
}

int closedir_inf(DIR *dp)
{
        int retval;

        if ((retval = closedir(dp))) {
		ERROR = errno;
                error(0, errno, "could not close directory");
	}
        return retval;
}

struct dirent *readdir_inf(DIR *dp)
{
        struct dirent *retval;

	/* 
	 * To distinguish between 
	 * end of a dir and error 
	 */
	errno = 0;

	if(!(retval = readdir(dp)) && errno) {
		ERROR = errno;
		error(0, errno, "could not read directory's entries");
	}
        return retval;
}

int unlink_inf(const char *pathname)
{
	int retval;

	if ((retval = unlink(pathname))) {
		ERROR = errno;
		error(0, errno, "could not remove file '%s'", pathname);
	}
	return retval;
}

int rmdir_inf(const char *pathname)
{
	int retval;

	if ((retval = rmdir(pathname))) {
		ERROR = errno;
		error(0, errno, "could not remove directory '%s'", pathname);
	}
	return retval;
}

FILE *fopen_inf(const char *path, const char *mode)
{
	FILE *fp;

	if (!(fp = fopen(path, mode))) {
		ERROR = errno;
		error(0, errno, "could not open file '%s'", path);
	}
	return fp;
}

int fclose_inf(FILE *fp)
{
	int retval;

	if ((retval = fclose(fp))) {
		ERROR = errno;
                error(0, errno, "could not close file");
	}
	return retval;
}

time_t time_inf(time_t *tloc)
{
	time_t retval;

	if ((retval = time(tloc)) == -1) {
		ERROR = errno;
		error(0, errno, "could not get time in seconds");
	}
	return retval;
}
