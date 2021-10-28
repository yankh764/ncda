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
#include "informative.h" 
#include "disk_analysis.h"


static inline void *alloc_stat()
{
        return malloc_inf(sizeof(struct stat));
}

static inline void free_and_null(void **ptr)
{
        free(*ptr);
        *ptr = NULL;
}

static void *alloc_bin_tree()
{
        struct bin_tree *node;

        if ((node = malloc_inf(sizeof(struct bin_tree)))) {
                /* 
                 * I decided to NULL the left and right child here
                 * so I don't mistakely forget to do that in further 
                 * functions
                 */
                node->left = NULL;
                node->right = NULL;
        }
        return node;
}

static void free_bin_tree(struct bin_tree *root, void (*free_data) (void *))
{
        if (root->left)
                free_bin_tree(root->left, free_data);
        if (root->right)
                free_bin_tree(root->right, free_data);
	free_data(root->data);
	free(root);
}

static char *get_entry_path(const char *dir_path, const char *entry_name)
{
        char *entry_path;
        size_t len;

	/* The 2 stands for a slash and a null byte */
	len = strlen(dir_path) + strlen(entry_name) + 2;
        
	if ((entry_path = malloc_inf(len)))
                snprintf(entry_path, len, "%s/%s", dir_path, entry_name);
        
	return entry_path;
}

static inline void *alloc_file_info()
{
        return malloc_inf(sizeof(struct file_info));
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

static inline void insert_rem_file_info_fields(struct file_info *info, 
					       const char *name, 
					       const size_t name_len,
					       const char *path,
					       const size_t path_len)
{
	memcpy(info->name, name, name_len);
	memcpy(info->path, path, path_len);
}

static int alloc_file_info_fields(struct file_info *ptr, 
				  const size_t name_len, 
				  const size_t path_len)
{	
	if (!(ptr->name = malloc_inf(name_len)))
		goto err_out;
	if (!(ptr->path = malloc_inf(path_len)))
		goto err_free_name;
	if (!(ptr->status = alloc_stat())) 
		goto err_free_path;

	return 0;

err_free_path:
	free(ptr->path);
err_free_name:
	free(ptr->name);
err_out:
	return -1;

}

static void free_file_info_fields(struct file_info *ptr)
{
	if (ptr->status)
		free(ptr->status);

	free(ptr->path);
	free(ptr->name);
}

static inline void free_file_info(struct file_info *ptr)
{
	free_file_info_fields(ptr);
	free(ptr);
}

static struct file_info *get_file_info(const char *path, const char *entry_name)
{
        const size_t name_len = strlen(entry_name) + 1;
        const size_t path_len = strlen(path) + 1;
        struct file_info *info;
	int ret;

        if ((info = alloc_file_info())) {
		if (alloc_file_info_fields(info, name_len, path_len))
			goto err_free_info;
		if ((ret = stat_custom_fail(path, info->status))) {
			if (ret == -1)
				goto err_free_info_fields;
			
			free_and_null((void **) &info->status);
		}
		insert_rem_file_info_fields(info, entry_name, 
					    name_len, path, path_len);
	}
        return info;

err_free_info_fields:
	free_file_info_fields(info);
err_free_info:
	free(info);
	
	return NULL;
}

static void get_desired_node(struct bin_tree **new_node, 
			     struct bin_tree **current)
{

}

/*
 * An helper function for get_dirs_content()
 */
static struct bin_tree *_get_dirs_content(DIR *dp, const char *dir_path)
{
        struct bin_tree *new_node;
        struct bin_tree *current;
        struct bin_tree *root;
        struct dirent *entry;
	char *entry_path;
        
	root = NULL;
	/* 
         * Reset errno to 0 to distnguish between 
         * error and end of directory in readdir_inf() 
         */
        errno = 0;

        while ((entry = readdir_inf(dp))) {
		if (!(new_node = alloc_bin_tree()))
			goto err_out;
		if (!root)
			current = root = new_node;
		else
			;
		if (!(entry_path = get_entry_path(dir_path, entry->d_name)))
			goto err_out;
		if (!(current->data = get_file_info(entry_path, entry->d_name)))
			goto err_out;
	}
}

static struct bin_tree *get_dirs_content(const char *path)
{
        struct bin_tree *root = NULL;
        DIR *dp;

        if ((dp = opendir_inf(path))) {
                root = _get_dirs_content(dp, path);
                
                if (closedir_inf(dp) && root)
                        free_bin_tree(root, free_file_info);
        }
        return root;
}
