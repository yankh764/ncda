/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for the disk management.                              |
---------------------------------------------------------
*/

/*
 * Defining _GNU_SOURCE macro since it achives all the desired
 * feature test macro requirements, which are:
 *     1) _XOPEN_SOURCE >= 500 || _ISOC99_SOURCE for snprintf() and lstat()
 *     2) _DEFAULT_SOURCE || _BSD_SOURCE for file type and mode macros
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "general.h"
#include "informative.h" 
#include "disk.h"

#define IGNORE_EACCES() (ERROR = 0)
/*
 * I defined these macros to get the appropriate entry color in an effiecient,
 * fast and clear way without making an external function call that will 
 * increase the overall execution time with the function-call overhead
 * (forcing the inline)
 */
#define IS_EXEC(file_mode)	      ((file_mode & S_IXUSR) || \
				       (file_mode & S_IXGRP) || \
				       (file_mode & S_IXOTH))

#define SHLD_BE_BLUE(file_mode)	       S_ISDIR(file_mode)
#define SHLD_BE_CYAN(file_mode)	       S_ISLNK(file_mode)
#define SHLD_BE_MAGENTA(file_mode)     S_ISSOCK(file_mode)
#define SHLD_BE_GREEN(file_mode)      (S_ISREG(file_mode) && IS_EXEC(file_mode))
#define SHLD_BE_YELLOW(file_mode)     (S_ISCHR(file_mode) || \
				       S_ISBLK(file_mode) || \
				       S_ISFIFO(file_mode))

/* Necessary static functions prototype */
static int rm_dir_r(const char *);


/*
 * Get entry's path in the case of dir_path being just a slash
 */
static char *get_entry_path_slash(const char *entry_name)
{
	char *entry_path;
	size_t len;

	/* 2 = the slash + null byte */
	len = strlen(entry_name) + 2;

	if ((entry_path = malloc_inf(len)))
		snprintf(entry_path, len, "/%s", entry_name);
	return entry_path;
}

/*
 * Get entry's path in the case of dir_path being just a regular
 * path and not just slash (contrast to get_entry_path_slash())
 */
static char *get_entry_path_not_slash(const char *dir_path, 
				      const char *entry_name)
{
	char *entry_path;
        size_t len;
	
	/* 2 = the slash + null byte */
	len = strlen(dir_path) + strlen(entry_name) + 2;
        
	if ((entry_path = malloc_inf(len)))
                snprintf(entry_path, len, "%s/%s", dir_path, entry_name);
	return entry_path;
}

static inline bool is_slash(const char *dir_path)
{
	return (strcmp(dir_path, "/") == 0);
}

static inline char *get_entry_path(const char *dir_path, const char *entry_name)
{
        if (is_slash(dir_path))
		return get_entry_path_slash(entry_name);
	else
		return get_entry_path_not_slash(dir_path, entry_name);
}

static void free_and_null_dtree(struct dtree **begin)
{
	free_dtree(*begin);
	*begin = NULL;
}

static int insert_fdata_fields(struct fdata *ptr, 
			       const char *entry_name, size_t nlen, 
			       const char *entry_path, size_t plen)
{
	int retval;
	
	if (!(retval = lstat_inf(entry_path, ptr->fstatus))) {
		memcpy(ptr->fname, entry_name, nlen);
		memcpy(ptr->fpath, entry_path, plen);
	}
	return retval;
}

static short proper_cpair(mode_t mode)
{
	if (SHLD_BE_BLUE(mode))
		return BLUE_PAIR;
	else if (SHLD_BE_GREEN(mode))
		return GREEN_PAIR;
	else if (SHLD_BE_CYAN(mode))
		return CYAN_PAIR;
	else if (SHLD_BE_YELLOW(mode)) 
		return YELLOW_PAIR;
	else if (SHLD_BE_MAGENTA(mode))
		return MAGENTA_PAIR;
	else
		return DEFAULT_PAIR;
}

/*
 * Get the proper end of a string. If the file is a directory the end of
 * the string will be slash ('/'), else the end of a string will be blank
 * character (' ').
 */
static inline char proper_eos(mode_t mode)
{
	return S_ISDIR(mode) ? '/' : ' ';
}

static inline void insert_cdata_fields(struct entry_data *ptr, int node_i)
{
	const int init_displayed_y = 2;

	ptr->curses->cpair = proper_cpair(ptr->file->fstatus->st_mode);
	ptr->curses->y = init_displayed_y + node_i;
	ptr->curses->eos = proper_eos(ptr->file->fstatus->st_mode);
}

static struct dtree *_get_entry_info(const char *entry_name, 
				     const char *entry_path,
				     int node_i)
{
	struct dtree *node;
	size_t plen, nlen;

	nlen = strlen(entry_name) + 1;
	plen = strlen(entry_path) + 1;

	if ((node = alloc_dtree(nlen, plen))) {
		if(!insert_fdata_fields(node->data->file, entry_name, nlen, entry_path, plen))
			insert_cdata_fields(node->data, node_i);
		else
			free_and_null_dtree(&node);
	}
	return node;
}

static struct dtree *get_entry_info(const char *dir_path, 
				    const char *entry_name,
				    int node_i) 
{
	struct dtree *node;
	char *entry_path;

	node = NULL;
	
	if ((entry_path = get_entry_path(dir_path, entry_name))) {
		node = _get_entry_info(entry_name, entry_path, node_i);
		free(entry_path);	
	}
	return node;
}

/*
 * Connect tow nodes together when they are directory 
 * mates (in the same directory).
 * The function returns the address of the new connected node
 */
static inline struct dtree *connect_mate_nodes(struct dtree *current, 
				               struct dtree *new_node)
{
	current->next = new_node;
	new_node->prev = current;

	return current->next;
}

/*
 * Connect tow nodes together when they are parent directory and 
 * child directory.
 * The function returns the address of the new connected node
 */
static inline struct dtree *connect_family_nodes(struct dtree *current, 
						 struct dtree *new_node)
{
	current->child = new_node;
	new_node->parent = current;

	return current->child;
}

static int lstat_custom_fail(const char *path, struct stat *statbuf)
{
        int retval;

        if ((retval = lstat_inf(path, statbuf)))
                if (ERROR == EACCES)
                        retval = IGNORE_EACCES();
        return retval;
}

/*
 * Return the address of the beggining (the dot entry) 
 */
static inline struct dtree *get_dot_entries(const char *dir_path)
{
	struct dtree *dot, *two_dots;
	const int tow_dots_i = 1;
	const int dot_i = 0;

	if ((dot = get_entry_info(dir_path, ".", dot_i))) {
		if ((two_dots = get_entry_info(dir_path, "..", tow_dots_i)))
			connect_mate_nodes(dot, two_dots);
		else 
			free_and_null_dtree(&dot);
	}
	return dot;
}

static struct dtree *_get_dir_tree(DIR *dp, const char *dir_path)
{
        struct dirent *entry;
        struct dtree *new_node;
        struct dtree *current;
        struct dtree *begin;
	struct dtree *child;
	int i;
	
	/* 
	 * I want the dot entries to be the first 2 nodes 
	 * of the doubly linked list so I'll add them seperately here 
	 * because there's no guarentee for the order in which file 
	 * names are read in readdir().
	 */
	if (!(begin = get_dot_entries(dir_path)))
		goto err_out;
	/* 
	 * The initial i is equal to 2 because i=0 goes to
	 * the dot entry and i=1 goes to the tow_dots entry
	 */
	i = 2;
	current = begin->next;

	while ((entry = readdir_inf(dp))) {
		if (is_dot_entry(entry->d_name))
			continue;
		if (!(new_node = get_entry_info(dir_path, entry->d_name, i++)))
			goto err_free_dtree;
		current = connect_mate_nodes(current, new_node);

		if (S_ISDIR(current->data->file->fstatus->st_mode)) {
			if ((child = get_dir_tree(current->data->file->fpath)))
				connect_family_nodes(current, child);
			else if (ERROR == EACCES)
				IGNORE_EACCES();
			else
				goto err_free_dtree;
		}
	}
	if (ERROR)
		goto err_free_dtree;

	return begin;

err_free_dtree:
	free_dtree(begin);
err_out:
	return NULL;
}

struct dtree *get_dir_tree(const char *path)
{
        struct dtree *begin;
        DIR *dp;

	begin = NULL;
	
        if ((dp = opendir_inf(path))) {
                begin = _get_dir_tree(dp, path);
                
                if (closedir_inf(dp) && begin)
                        free_and_null_dtree(&begin);
        }
        return begin;
}

static int unlink_custom_fail(const char *path)
{
	int retval;

	if ((retval = unlink_inf(path)))
		if (ERROR == EACCES)
			retval = IGNORE_EACCES();
	return retval;
}

static int rmdir_custom_fail(const char *path)
{
	int retval;

	if ((retval = rmdir_inf(path)))
		if (ERROR == EACCES)
			retval = IGNORE_EACCES();
	return retval;
}

static int delete_entry_use_lstat(const char *entry_path)
{
	struct stat statbuf;

	 if (lstat_custom_fail(entry_path, &statbuf))
		 return -1;
	 if (S_ISDIR(statbuf.st_mode))
		return rm_dir_r(entry_path);
	 else
		 return unlink_custom_fail(entry_path);
}

static inline int delete_entry_use_d_type(const char *entry_path, 
					  unsigned char d_type)
{
	if (d_type == DT_DIR)
		return rm_dir_r(entry_path);
	else
		return unlink_custom_fail(entry_path);
}

static inline int _delete_entry(const char *entry_path, unsigned char d_type)
{
	if (d_type == DT_UNKNOWN)
		return delete_entry_use_lstat(entry_path);
	else
		return delete_entry_use_d_type(entry_path, d_type);
}

static int delete_entry(const char *dir_path, 
		        const char *entry_name, 
			unsigned char d_type)
{
	char *entry_path;	
	int retval;
	
	retval = -1;
	if ((entry_path = get_entry_path(dir_path, entry_name))) {
		retval = _delete_entry(entry_path, d_type);
		free(entry_path);
	}
	return retval;
}

static int _rm_dir_r(DIR *dp, const char *path)
{
	struct dirent *entry;
	
	while ((entry = readdir_inf(dp))) {
		if (is_dot_entry(entry->d_name))
			continue;
		if (delete_entry(path, entry->d_name, entry->d_type))
			return -1;
	}
	return ERROR ? -1 : 0;
}

/*
 * Remove directory and it's content (recursively)
 */
static int rm_dir_r(const char *path)
{
	int retval;
	DIR *dp;

	retval = -1;

	if ((dp = opendir_inf(path))) {
		retval = _rm_dir_r(dp, path);
		
		if (closedir_inf(dp))
			retval = -1;
		/* Remove the directory itself if it's already empty */
		if (!retval)
			retval = rmdir_custom_fail(path);
	} else if (ERROR == EACCES) {
		retval = IGNORE_EACCES();
	}
	return retval;
}

int rm_entry(const struct fdata *data)
{
	if (S_ISDIR(data->fstatus->st_mode))
		return rm_dir_r(data->fpath);
	else
		return unlink_custom_fail(data->fpath);
}

static inline bool is_zero_sized(off_t size)
{
	return size == 0;
}

static inline bool is_relevant_dir_entry(const struct stat *statbuf)
{
	return S_ISDIR(statbuf->st_mode) && !is_zero_sized(statbuf->st_size);
}

/*
 * Get the acctual directory size (the size of it's content)
 */
static off_t get_acc_dir_size(struct dtree *dir_ptr)
{
	struct dtree *first_entry;
	struct dtree *current;
	off_t total;

	first_entry = dir_ptr->child;

	for (current=first_entry, total=0; current; current=current->next) {
		/* Zero size dot entries so they won't be relevant */
		if (is_dot_entry(current->data->file->fname))
			current->data->file->fstatus->st_size = 0;
		else if (is_relevant_dir_entry(current->data->file->fstatus))
			current->data->file->fstatus->st_size = get_acc_dir_size(current);
		total += current->data->file->fstatus->st_size;
	}
	return total;
}

/*
 * Correct the st_size fields for the directories since it represents 
 * only the entry size and not with the actual directory's content size.
 */
void correct_dtree_st_size(struct dtree *begin)
{
	struct dtree *current;

	for (current=begin; current; current=current->next) {
		/* Zero dot entries' size so they won't be relevant */
		if (is_dot_entry(current->data->file->fname))
			current->data->file->fstatus->st_size = 0;
		else if (is_relevant_dir_entry(current->data->file->fstatus))
			current->data->file->fstatus->st_size = get_acc_dir_size(current);	
	}
}