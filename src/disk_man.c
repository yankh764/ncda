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
#include "informative.h" 
#include "disk_man.h"


/* Necessary static functions prototype */
static int rm_dir_r(const char *);
static off_t get_dir_size(const char *);


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
	int retval;
	
	if (!(retval = lstat(entry_path, ptr->fstatus)))
		insert_fdata_fields(ptr, entry_name, nlen, entry_path, plen);
	return retval;
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

bool is_dot_entry(const char *entry_name)
{
	return (strcmp(entry_name, ".") == 0 ||
		strcmp(entry_name, "..") == 0);
}

static inline void connect_nodes(struct doubly_list *current, 
				 struct doubly_list *new_node)
{
	current->next = new_node;
	new_node->prev = current;
}

static inline void connect_dot_entries(struct doubly_list *dot, 
				       struct doubly_list *two_dots, 
				       struct doubly_list *head)
{
	connect_nodes(dot, two_dots);
	if (head)
		connect_nodes(two_dots, head);
}

static inline int ignore_eacces()
{
	errno = 0;
	return 0;
}

static int lstat_custom_fail(const char *path, struct stat *statbuf)
{
        int retval;

        if ((retval = lstat_inf(path, statbuf)))
                if (errno == EACCES)
                        retval = ignore_eacces();
        return retval;
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

/*
 * A helper function for get_dirs_content()
 */
static struct doubly_list *_get_dirs_content(DIR *dp, const char *dir_path)
{
        struct dirent *entry;
        struct doubly_list *new_node;
        struct doubly_list *new_head;
        struct doubly_list *current;
        struct doubly_list *head;
        
	head = NULL;
        errno = 0;

	while ((entry = readdir_inf(dp))) {
		if (is_dot_entry(entry->d_name))
			continue;
		if (!(new_node = get_doubly_list_node(dir_path, entry->d_name)))
			goto err_free_doubly_list;
		if (head) {
			connect_nodes(current, new_node);
			current = current->next;
		} else {
			current = head = new_node;
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
	if (!(new_head = prepend_dot_entries(dir_path, head)))
		goto err_free_doubly_list;

	return new_head;

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

static inline int _delete_entry_use_lstat(const char *entry_path)
{
	struct stat statbuf;

	 if (lstat_inf(entry_path, &statbuf))
		 return -1;
	 if (S_ISDIR(statbuf.st_mode))
		return rm_dir_r(entry_path);
	 else
		 return rm_file(entry_path);
}

static inline int _delete_entry_use_d_type(const char *entry_path, 
					   unsigned char d_type)
{
	if (d_type == DT_DIR)
		return rm_dir_r(entry_path);
	else
		return rm_file(entry_path);
}

static int _delete_entry(const char *entry_path, unsigned char d_type)
{
	if (d_type == DT_UNKNOWN)
		return _delete_entry_use_lstat(entry_path);
	else
		return _delete_entry_use_d_type(entry_path, d_type);
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
	
	/* 
	 * Reset errno to 0 to distnguish between 
	 * error and end of directory in readdir_inf() 
	 */
	errno = 0;

	while ((entry = readdir_inf(dp))) {
		if (is_dot_entry(entry->d_name))
			continue;
		if (delete_entry(path, entry->d_name, entry->d_type))
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

int rm_entry(const struct fdata *data)
{
	if (S_ISDIR(data->fstatus->st_mode))
		return rm_dir_r(data->fpath);
	else
		return rm_file(data->fpath);
}

off_t get_total_disk_usage(const struct doubly_list *head)
{
	const struct doubly_list *current;
	off_t total;

	for (current=head, total=0; current; current=current->next)
		total += current->data->file_data->fstatus->st_size;
	return total;
}

static inline struct size_format proper_size_format(float size, char *unit)
{
	struct size_format retval;

	retval.size_format = size;
	retval.size_unit = unit;

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

static inline off_t get_size_use_lstat(const struct dirent *entry, 
				       const char *entry_path)
{
	struct stat statbuf;

	if (lstat_custom_fail(entry_path, &statbuf))
		return -1;
	if (S_ISDIR(statbuf.st_mode) && !is_dot_entry(entry->d_name))
		return get_dir_size(entry_path);
	else	
		return statbuf.st_size;

}

static off_t _get_dir_size(DIR *dp, const char *path)
{
	struct dirent *entry;
	char *entry_path;
	off_t size;
	off_t ret;

	errno = 0;
	size = 0;

	while ((entry = readdir_inf(dp))) {
		if (!(entry_path = get_entry_path(path, entry->d_name)))
			break;
		if (entry->d_type == DT_DIR && !is_dot_entry(entry->d_name)) {
			if ((ret = get_dir_size(entry_path)) == -1)
				goto err_free_entry_path;
			size += ret;	
		} else {
			if ((ret = get_size_use_lstat(entry, entry_path)) == -1)
				goto err_free_entry_path;
			size += ret;
		}
		free(entry_path);
	}
	return errno ? -1 : size;

err_free_entry_path:
	free(entry_path);
	
	return -1;
}

static off_t get_dir_size(const char *path)
{
	off_t retval;
	DIR *dp;

	retval = -1;

	if ((dp = opendir_inf(path))) {
		retval = _get_dir_size(dp, path);

		if (closedir_inf(dp) && retval != -1)
			retval = -1;
	} else if (errno == EACCES) {
		retval = ignore_eacces();
	}
	return retval;
}

static inline int correct_st_size(struct fdata *ptr)
{
	off_t size;

	if ((size = get_dir_size(ptr->fpath)) == -1)
		return -1;
	ptr->fstatus->st_size += size;
	
	return 0;
}

static inline bool is_proc(const char *path)
{
	return (strcmp(path, "proc") == 0);
}

static inline bool is_unsupported_dir(const char *dir_name)
{
	/* 
	 * Since /proc is virtual file system and owned by the kernel, 
	 * it may be problrmatic to analayze and it could create some problems
	 */
	return is_proc(dir_name) || is_dot_entry(dir_name);
}

/*
 * Correct the st_size fields for the directories since it represents 
 * only the entry size and not with the actual directory's content size
 */
int correct_dir_st_size(struct doubly_list *head)
{
	struct doubly_list *current;
	int retval;

	retval = 0;

	for (current=head; current; current=current->next)
		if (S_ISDIR(current->data->file_data->fstatus->st_mode) &&
		    !is_unsupported_dir(current->data->file_data->fname))
			if ((retval = correct_st_size(current->data->file_data)))
				break;
	return retval;
}
