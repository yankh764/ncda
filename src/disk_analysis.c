/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for the disk analysis.                                |
---------------------------------------------------------
*/

/*
 * Defining _GNU_SOURCE macro since it achives all the desired
 * feature test macro requirements, which are:
 *     1) _XOPEN_SOURCE >= 500 || _ISOC99_SOURCE for snprintf()
 *     2) _DEFAULT_SOURCE || _BSD_SOURCE for d_type field in struct dirent
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "curses.h"
#include "informative.h" 
#include "disk_analysis.h"


static inline void free_and_null(void **ptr)
{
        free(*ptr);
        *ptr = NULL;
}

static char *_get_entry_path_slash(const char *entry_name)
{
	char *entry_path;
	size_t len;

	len = strlen(entry_name) + 2;

	if ((entry_path = malloc_inf(len)))
		snprintf(entry_path, len, "/%s", entry_name);
	
	return entry_path;
}

static char *_get_entry_path_not_slash(const char *dir_path, 
				       const char *entry_name)
{
	char *entry_path;
        size_t len;
	
	len = strlen(dir_path) + strlen(entry_name) + 2;
        
	if ((entry_path = malloc_inf(len)))
                snprintf(entry_path, len, "%s/%s", dir_path, entry_name);
        
	return entry_path;
}

static inline int is_slash(const char *dir_path)
{
	return (strcmp(dir_path, "/") == 0);
}

static char *get_entry_path(const char *dir_path, const char *entry_name)
{
        if (is_slash(dir_path))
		return _get_entry_path_slash(entry_name);
	else
		return _get_entry_path_not_slash(dir_path, entry_name);
}

/*
 * The fucntion uses stat() to get files status but it 
 * will not consider permission denied (EACCES) error as a failure.
 */
static int stat_custom_fail(const char *path, struct stat *statbuf)
{
        int retval;

        if ((retval = stat_inf(path, statbuf)))
                if (errno == EACCES)
                        retval = EACCES;
        return retval;
}

/*
 * Insert the remaining fdata fields that werent inserted
 */
static inline void insert_fdata_fields(struct fdata *ptr, const char *name, 
				       size_t nlen, const char *path, 
				       size_t plen)
{
	memcpy(ptr->fname, name, nlen);
	memcpy(ptr->fpath, path, plen);
}

static struct fdata *get_fdata(const char *path, const char *entry_name)
{
        const size_t name_len = strlen(entry_name) + 1;
        const size_t path_len = strlen(path) + 1;
        struct fdata *data;
	int ret;

        if ((data = alloc_fdata(name_len, path_len))) {
		if ((ret = stat_custom_fail(path, data->fstatus))) {
			if (ret == -1)
				goto err_free_fdata;
			
			free_and_null((void **) &data->fstatus);
		}
		insert_fdata_fields(data, entry_name, name_len, path, path_len);
	}
        return data;

err_free_fdata:
	free_fdata(data);
	
	return NULL;
}

/*
 * Decide which node between root->left or root->right
 *
static struct bin_tree *decide_which_node(struct bin_tree *root, 
					  struct bin_tree *new_node)
{
	struct bin_tree *retval;

	if (root->left)
		retval = root->right = new_node;
	else
		retval = root->left = new_node;

	return retval;
}


static struct bin_tree *get_current_node(struct bin_tree *root, 
					 struct bin_tree *new_node, 
					 const unsigned int parent)
{
	struct bin_tree *ret;

	if (root->data->node_num == parent) {
		return decide_which_node(root, new_node);
	} else if (root->left) {
		if ((ret = get_current_node(root->left, new_node, parent)))
			return ret;
	} else if (root->right) {
		if ((ret = get_current_node(root->right, new_node, parent)))
			return ret;
	}
	return NULL;
}

static inline unsigned int get_parent_i(const unsigned int i)
{
	return ((i-1) / 2);
}
*/

static inline void free_and_null_list(struct list **head)
{
	free_list(*head);
	*head = NULL;
}

static inline bool is_dot_entry(const char *entry_name)
{
	return (strcmp(entry_name, ".") == 0 ||
		strcmp(entry_name, "..") == 0);
}

/*
 * A helper function for get_dirs_content()
 */
static struct list *_get_dirs_content(DIR *dp, const char *dir_path)
{
        struct dirent *entry;
        struct list *new_node;
        struct list *current;
        struct list *head;
	char *entry_path;
        
	head = NULL;
	/* 
         * Reset errno to 0 to distnguish between 
         * error and end of directory in readdir_inf() 
         */
        errno = 0;

	while ((entry = readdir_inf(dp))) {
		if (is_dot_entry(entry->d_name))
			continue;
		if (!(new_node = alloc_list()))
			goto err_free_list;
		if (!head)
			current = head = new_node;
		else
			current = current->next = new_node;
		if (!(entry_path = get_entry_path(dir_path, entry->d_name)))
			goto err_free_list;
		if (!(current->data = get_fdata(entry_path, entry->d_name)))
			goto err_free_entry_path;
		free(entry_path);
	}
	if (errno)
		goto err_free_list;

	return head;

err_free_entry_path:
	free(entry_path);
err_free_list:
	if (head)
		free_list(head);

	return NULL;
}

struct list *get_dirs_content(const char *path)
{
        struct list *head;
        DIR *dp;

	head = NULL;
        if ((dp = opendir_inf(path))) {
                head = _get_dirs_content(dp, path);
                
                if (closedir_inf(dp) && head)
                        free_and_null_list(&head);
        }
        return head;
}
