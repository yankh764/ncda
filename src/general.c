/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for all the general fucntions in the ncda program     |
---------------------------------------------------------
*/

/*
 * Defining _GNU_SOURCE macro since it achives all the desired
 * feature test macro requirements, which are:
 *     1) _XOPEN_SOURCE >= 500 || _ISOC99_SOURCE for snprintf()
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "informative.h"
#include "general.h"


void free_and_null(void **ptr)
{
	free(*ptr);
	*ptr = NULL;
}

/*
 * Return zero if s1 is equal to s2. Else a non-zero value
 * will be returned.
 */
int efficient_strcmp(const char *s1, const char *s2)
{
	/* 
	 * Since the fact that I'm going to compare lots
	 * paths the first character is going to be the same
	 * which is a slash, therfore I will compare the second
	 * character of the paths.
	 */
	if (s1[0] == s2[0] && s1[1] == s2[1])
		return strcmp(s1, s2);
	else
		return -1;
}

bool is_dot_entry(const char *entry_name)
{
	return (efficient_strcmp(entry_name, ".") == 0 ||
		efficient_strcmp(entry_name, "..") == 0);
}

static struct size_format proper_size_format(float size, char *unit)
{
	struct size_format retval;

	retval.val = size;
	retval.unit = unit;

	return retval;
}

static inline float bytes_to_tb(off_t bytes)
{
	return bytes / 1000000000000.0;
}

static inline float bytes_to_gb(off_t bytes)
{
	return bytes / 1000000000.0;
}

static inline float bytes_to_mb(off_t bytes)
{
	return bytes / 1000000.0;
}

static inline float bytes_to_kb(off_t bytes)
{
	return bytes / 1000.0;
}

struct size_format get_proper_size_format(off_t bytes)
{
	const off_t tb = 1000000000000;
	const off_t gb = 1000000000;
	const off_t mb = 1000000;
	const off_t kb = 1000;

	if (bytes >= tb)
		return proper_size_format(bytes_to_tb(bytes), "TB");
	else if (bytes >= gb)
		return proper_size_format(bytes_to_gb(bytes), "GB");
	else if (bytes >= mb)
		return proper_size_format(bytes_to_mb(bytes), "MB");
	else if (bytes >= kb)
		return proper_size_format(bytes_to_kb(bytes), "KB");
	else
		return proper_size_format(bytes, "B");
}

static void remove_plural_s(char *str)
{
	size_t len;

	len = strlen(str);
	str[--len] = '\0';
}

static char *_mtime_str(unsigned int num, char *period)
{
	char *retval;
	size_t size;	

	if (num == 1)
		remove_plural_s(period);
	size = sizeof(num) + strlen(period) + sizeof(' ') + 1;
	
	if ((retval = malloc_inf(size)))
		snprintf(retval, size, "%2d %s", num, period);
	return retval;
}

static inline char *mtime_str_century(unsigned int num)
{
	char period[] = "centuries";

	return _mtime_str(num, period);
}

static inline char *mtime_str_decade(unsigned int num)
{
	char period[] = "decades";

	return _mtime_str(num, period);
}

static inline char *mtime_str_year(unsigned int num)
{
	char period[] = "years";

	return _mtime_str(num, period);
}

static inline char *mtime_str_month(unsigned int num)
{
	char period[] = "months";

	return _mtime_str(num, period);
}

static inline char *mtime_str_week(unsigned int num)
{
	char period[] = "weeks";

	return _mtime_str(num, period);
}

static inline char *mtime_str_day(unsigned int num)
{
	char period[] = "days";

	return _mtime_str(num, period);
}
 
static char *mtime_str(time_t difference)
{
	const time_t sec_in_day = 60 * 60 * 24;
	const time_t century = sec_in_day * 365 * 100;
	const time_t decade = sec_in_day * 365 * 10;
	const time_t year = sec_in_day * 365;
	const time_t month = sec_in_day * 30;
	const time_t week = sec_in_day * 7;

	if (difference >= century)
		return mtime_str_century(difference/century);
	else if (difference >= decade)
		return mtime_str_decade(difference/decade);
	else if (difference >= year)
		return mtime_str_year(difference/year);
	else if (difference >= month)
		return mtime_str_month(difference/month);
	else if (difference >= week)
		return mtime_str_week(difference/week);
	else
		return mtime_str_day(difference/sec_in_day);
}

char *get_mtime_str(time_t mtime)
{
	time_t today;

	if ((today = time_inf(NULL)) != -1)
		return mtime_str(today-mtime);
	else
		return NULL;
}

size_t get_strsize(const char *str)
{
	const char null_byte = '\0';

	return strlen(str) + sizeof(null_byte);
}

/*
 * Extract directory's path from the first entry path (dot_entry)
 */
char *extract_dir_path(const char *dot_entry_path)
{
	size_t len;
	char *path;

	len = get_strsize(dot_entry_path);
	
	if (len > 2)
		len--;
	if ((path = malloc_inf(len))) {
		memcpy(path, dot_entry_path, len);
		path[--len] = '\0';
	}
	return path;
} 
/**/