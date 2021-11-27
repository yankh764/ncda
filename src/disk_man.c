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
#include "informative.h" 
#include "disk_man.h"


/* Necessary static functions prototype */
static int rm_dir_r(const char *);


static inline void free_and_null(void **ptr)
{
        free(*ptr);
        *ptr = NULL;
}

/*
 * Get entry's path in the case of dir_path being just a slash
 */
static char *get_entry_path_slash(const char *entry_name)
{
	char *entry_path;
	size_t len;

	len = strlen(entry_name) + 2;

	if ((entry_path = malloc_inf(len)))
		snprintf(entry_path, len, "/%s", entry_name);
	return entry_path;
}

/*
 * Get entry's path in the case of dir_path being just a regular
 * path and not just slash (contrast to _get_entry_path_slash())
 */
static char *get_entry_path_not_slash(const char *dir_path, 
				      const char *entry_name)
{
	char *entry_path;
        size_t len;
	
	len = strlen(dir_path) + strlen(entry_name) + 2;
        
	if ((entry_path = malloc_inf(len)))
                snprintf(entry_path, len, "%s/%s", dir_path, entry_name);
	return entry_path;
}

static inline bool is_slash(const char *dir_path)
{
	return (strcmp(dir_path, "/") == 0);
}

static char *get_entry_path(const char *dir_path, const char *entry_name)
{
        if (is_slash(dir_path))
		return get_entry_path_slash(entry_name);
	else
		return get_entry_path_not_slash(dir_path, entry_name);
}

/*
 * The fucntion uses lstat() to get files status but it 
 * will not consider permission denied (EACCES) error as a failure.
 */
static int lstat_custom_fail(const char *path, struct stat *statbuf)
{
        int retval;

        if ((retval = lstat_inf(path, statbuf)))
                if (errno == EACCES)
                        retval = EACCES;
        return retval;
}

static inline void free_and_null_doubly_list(struct doubly_list **head)
{
	free_doubly_list(*head);
	*head = NULL;
}

static inline void insert_fdata_fields(struct fdata *ptr, 
				       const char *name, size_t nlen, 
				       const char *path, size_t plen)
{
	memcpy(ptr->fname, name, nlen);
	memcpy(ptr->fpath, path, plen);
}

static int get_fdata_fields(struct fdata *ptr, 
			    const char *entry_path, size_t plen,
			    const char *entry_name, size_t nlen)
{
	int ret;

	if ((ret = lstat_custom_fail(entry_path, ptr->fstatus))) {
		if (ret == -1)
			return -1;
		free_and_null((void **) &ptr->fstatus);
	}
	insert_fdata_fields(ptr, entry_name, nlen, entry_path, plen);

	return 0;
}

static struct doubly_list *_get_doubly_list_node(const char *entry_path,
						 const char *entry_name)
{
	size_t path_len, name_len;
	struct doubly_list *node;

	name_len = strlen(entry_name) + 1;
	path_len = strlen(entry_path) + 1;

	if ((node = alloc_doubly_list(name_len, path_len)))
		if(get_fdata_fields(node->data->file_data, 
				    entry_path, path_len,
				    entry_name, name_len))
			free_and_null_doubly_list(&node);
	return node;
}

static struct doubly_list *get_doubly_list_node(const char *dir_path, 
						const char *entry_name) 
{
	struct doubly_list *node;
	char *entry_path;

	node = NULL;
	if ((entry_path = get_entry_path(dir_path, entry_name))) {
		node = _get_doubly_list_node(entry_path, entry_name) ;
		free(entry_path);	
	}
	return node;
}

static inline bool is_dot_entry(const char *entry_name)
{
	return (strcmp(entry_name, ".") == 0 ||
		strcmp(entry_name, "..") == 0);
}

static inline void connect_dot_entries(struct doubly_list *dot, 
				       struct doubly_list *two_dots, 
				       struct doubly_list *head)
{
	dot->next = two_dots;
	two_dots->next = head;
	head->prev = two_dots;
	two_dots->prev = dot;
}

static struct doubly_list *prepend_dot_entries(const char *dir_path, 
					       struct doubly_list *head)
{
	struct doubly_list *two_dots;
	struct doubly_list *dot;

	if ((dot = get_doubly_list_node(dir_path, "."))) {
		if ((two_dots = get_doubly_list_node(dir_path, "..")))
			connect_dot_entries(dot, two_dots, head);
		else 
			free_and_null_doubly_list(&dot);
		
	}
	return dot;
}

static inline void connect_nodes(struct doubly_list *current, 
				 struct doubly_list *new_node)
{
	current->next = new_node;
	new_node->prev = current;
}

/*
 * A helper function for get_dirs_content()
 */
static struct doubly_list *_get_dirs_content(DIR *dp, const char *dir_path)
{
        struct dirent *entry;
        struct doubly_list *new_node;
        struct doubly_list *current;
        struct doubly_list *head;
        
	head = NULL;
        errno = 0;

	while ((entry = readdir_inf(dp))) {
		if (is_dot_entry(entry->d_name))
			continue;
		if (!(new_node = get_doubly_list_node(dir_path, entry->d_name)))
			goto err_free_doubly_list;
		if (!head) {
			current = head = new_node;
		} else {
			connect_nodes(current, new_node);
			current = current->next;
		}
	}
	if (errno)
		goto err_free_doubly_list;

	/* 
	 * I want the dot entries to be the first 2 nodes 
	 * of the doubly linked list so I skipped them in the 
	 * while loop because there's no guarentee for the order 
	 * in which file names are read in readdir(). So I decided
	 * to prepend them to the head node at the end of the function.
	 */
	return prepend_dot_entries(dir_path, head);

err_free_doubly_list:
	if (head)
		free_doubly_list(head);
	
	return NULL;
}

struct doubly_list *get_dirs_content(const char *path)
{
        struct doubly_list *head;
        DIR *dp;

	head = NULL;
        if ((dp = opendir_inf(path))) {
                head = _get_dirs_content(dp, path);
                
                if (closedir_inf(dp) && head)
                        free_and_null_doubly_list(&head);
        }
        return head;
}

static inline int rm_file(const char *path)
{
	return unlink_inf(path);
}

static int _delete_entry(const char *entry_path)
{
	struct stat statbuf;

	if (lstat_inf(entry_path, &statbuf)) 
		return -1;
	if (S_ISDIR(statbuf.st_mode))
		return rm_dir_r(entry_path);
	else
		return rm_file(entry_path);
}

static int delete_entry(const char *dir_path, const char *entry_name)
{
	char *entry_path;	
	int retval;
	
	retval = -1;
	if ((entry_path = get_entry_path(dir_path, entry_name))) {
		retval = _delete_entry(entry_path);
		free(entry_path);
	}
	return retval;
}

static int _rm_dir_r(DIR *dp, const char *path)
{
	struct dirent *entry;
	
	/* 
	 * Reset errno to 0 to distnguish between 
	 * error and end of directory in readdir_inf() 
	 */
	errno = 0;

	while ((entry = readdir_inf(dp))) {
		if (is_dot_entry(entry->d_name))
			continue;
		if (delete_entry(path, entry->d_name))
			break;
	}
	return errno ? -1 : 0;
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
		/* Remove directory's content */
		retval = _rm_dir_r(dp, path);
		
		if (closedir_inf(dp))
			retval = -1;
		if (!retval)
			/* Remove the directory itself */
			retval = rmdir_inf(path);
	}
	return retval;
}

int rm_entry(struct fdata *const data)
{
	if (S_ISDIR(data->fstatus->st_mode))
		return rm_dir_r(data->fpath);
	else
		return rm_file(data->fpath);
}


