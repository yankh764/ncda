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

/*
static void *alloc_stat()
{
        return malloc_inf(sizeof(struct stat));
}

static struct stat *get_stat(const int fd)
{
        struct stat *statbuf;

        if ((statbuf = alloc_stat())
                if (fstat_inf(fd, statbuf))
                        free_and_null(&statbuf);

        return statbuf;
}
*/

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

static void free_bin_tree(struct bin_tree *root)
{
        if (root->left)
                free_bin_tree(root->left);
        if (root->right)
                free_bin_tree(root->right);
        
        free_bin_tree(root);
}

static char *get_entry_path(const char *dir_path, const char *entry_name)
{
        const size_t len = strlen(dir_path) + strlen(entry_name) + 2;
        char *entry_path;

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
 * will not consider permission denied error as a failure.
 */
static int stat_custom_fail(const char *path, struct file_info *info)
{
        int retval;

        if ((retval = stat_inf(path, info->status))) 
                if (errno == EACCES) {
                        retval = 0;
                        info->status = NULL;
                }
        return retval;
}

/*
 * Insert the remaining file_info fields to the structure which 
 * are file_info name, and file_info path.
 */
static inline void insert_rem_file_info(struct file_info *info, 
                                        const char *path, 
                                        const char *entry_name,
                                        size_t entry_name_len)
{
        memcpy(info->name, entry_name, entry_name_len);
        info->path = (char *) path;
}

static struct file_info *get_file_info(const char *path, const char *entry_name)
{
        const size_t len = strlen(entry_name) + 1;
        struct file_info *info;

        if ((info = alloc_file_info())) {
                if (stat_custom_fail(path, info) 
		  || !(info->name = malloc_inf(len)))
                        free_and_null((void **) &info);
                else 
                        insert_rem_file_info(info, path, entry_name, len); 
        }
        return info;
}

static struct bin_tree *get_right_node(struct bin_tree **root)
{
	if (!*root)
		return root;

}

/*
 * An helper function for get_dirs_content()
 */
static struct bin_tree *_get_dirs_content(DIR *dp, const char *dir_path)
{
        struct bin_tree *current;
        struct bin_tree *root;
        struct dirent *entry;
	char *entry_path;
        
       root = current = NULL;
	/* 
         * Reset errno to 0 to distnguish between 
         * error and end of directory in readdir_inf() 
         */
        errno = 0;

        while ((entry = readdir_inf(dp))) {
		if (!(current = alloc_bin_tree()))
			goto err_out;
		if (!root)
			root = current;
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
                        free_bin_tree(root);
        }
        return root;
}
