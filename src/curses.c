/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for managing the ncurses windows                      |
---------------------------------------------------------
*/

/*
 * Defining _GNU_SOURCE macro since it achives all the desired
 * feature test macro requirements, which are:
 *     1) _DEFAULT_SOURCE || _BSD_SOURCE for file type and mode macros
 */
#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "disk_man.h"
#include "curses.h"

/*
 * I defined these macros to get the appropriate entry color in an effiecient,
 * fast and clear way without making an external function call that will 
 * increase the overall execution time with the function-call overhead.
 */
#define IS_EXEC(file_mode)	      ((file_mode & S_IXUSR) || \
				       (file_mode & S_IXGRP) || \
				       (file_mode & S_IXOTH))

#define SHLD_BE_BLUE(file_mode)	       S_ISDIR(file_mode)
#define SHLD_BE_CYAN(file_mode)	       S_ISLNK(file_mode)
#define SHLD_BE_MAGENTA(file_mode)     S_ISSOCK(file_mode)
#define SHLD_BE_GREEN(file_mode)       S_ISREG(file_mode) && IS_EXEC(file_mode)
#define SHLD_BE_YELLOW(file_mode)     (S_ISCHR(file_mode) || \
				       S_ISBLK(file_mode) || \
				       S_ISFIFO(file_mode))

enum COLOR_PAIRS_NUM {
	BLUE_PAIR =  1,
	GREEN_PAIR = 2, 
	YELLOW_PAIR = 3,
	CYAN_PAIR = 4,
	MAGENTA_PAIR = 5,
	DEFAULT_PAIR = 6
};

struct size_format {
	float size_format;
	char *size_unit;
};

struct doubly_list *highligted_node; 


static short get_color_pair_num(mode_t st_mode)
{
	if (SHLD_BE_BLUE(st_mode))
		return BLUE_PAIR;
	else if (SHLD_BE_GREEN(st_mode))
		return GREEN_PAIR;
	else if (SHLD_BE_CYAN(st_mode))
		return CYAN_PAIR;
	else if (SHLD_BE_YELLOW(st_mode)) 
		return YELLOW_PAIR;
	else if (SHLD_BE_MAGENTA(st_mode))
		return MAGENTA_PAIR;
	else
		return DEFAULT_PAIR;
}

static inline short proper_color_pair(struct stat *statbuf)
{
	if (COLORED_OUTPUT && statbuf) 
		return get_color_pair_num(statbuf->st_mode);
	else 
		return 0;
}

/*
 * Get the proper end of a string. If the file is a directory the end of
 * the string will be slash ('/'), else the end of a string will be blank
 * character (' ').
 */
static inline char proper_eos(struct stat *statbuf)
{
	return (statbuf && S_ISDIR(statbuf->st_mode)) ? '/' : ' ';
}

static inline void insert_cdata_fields(struct doubly_list *ptr, int i)
{
	struct stat *statbuf = ptr->data->file_data->fstatus;

	ptr->data->curses_data->cpair = proper_color_pair(statbuf);
	/* The 2 = an empty line + border */
	ptr->data->curses_data->y = i + 2;
	ptr->data->curses_data->eos = proper_eos(statbuf);
}

void nc_get_cdata_fields(struct doubly_list *head)
{
	struct doubly_list *current;
	unsigned int i;
	
	for (current=head, i=0; current; current=current->next, i++)
		insert_cdata_fields(current, i);
}

/*
 * This function was created for the sake of readability (to avoid reading
 * all the struct derefrencings inside bigger functions)
 */
static inline int attron_color(WINDOW *wp, struct doubly_list *const node)
{
	short color_pair = node->data->curses_data->cpair;

	return (wattron(wp, COLOR_PAIR(color_pair)) == ERR) ? -1 : 0;
}

/*
 * This function was created for the sake of readability (to avoid reading
 * all the struct derefrencings inside bigger functions and making a mess)
 */
static inline int print_fname(WINDOW *wp, struct doubly_list *const node) 
{
	const char *name = node->data->file_data->fname; 
	char eos = node->data->curses_data->eos;
	int y = node->data->curses_data->y;

	return (mvwprintw(wp, y, 0, "%s%c\n", name, eos) == ERR) ? -1 : 0;
}

static int display_fname_color(WINDOW *wp, struct doubly_list *const head)
{
	struct doubly_list *current;

	for (current=head; current; current=current->next) {
		if (attron_color(wp, current))
			return -1;
		if (print_fname(wp, current))
			return -1;
	}
	/* Reset the colors */
	return (wattron(wp, COLOR_PAIR(DEFAULT_PAIR)) == ERR) ? -1 : 0;
}

static int display_fname_no_color(WINDOW *wp, struct doubly_list *const head)
{
	struct doubly_list *current;

	for (current=head; current; current=current->next)
		if (print_fname(wp, current))
			return -1;
	return 0;
}

int nc_display_fname(WINDOW *wp, struct doubly_list *const head) 
{
	if (COLORED_OUTPUT)
		return display_fname_color(wp, head);
	else
		return display_fname_no_color(wp, head);
}

static inline int init_color_pairs()
{
	return (init_pair(BLUE_PAIR,    COLOR_BLUE,    -1) == ERR ||
	        init_pair(GREEN_PAIR,   COLOR_GREEN,   -1) == ERR ||
	        init_pair(YELLOW_PAIR,  COLOR_YELLOW,  -1) == ERR ||
	        init_pair(CYAN_PAIR,    COLOR_CYAN,    -1) == ERR ||
	        init_pair(MAGENTA_PAIR, COLOR_MAGENTA, -1) == ERR ||
	        init_pair(DEFAULT_PAIR, -1,            -1) == ERR) ? -1 : 0;
}

static int start_ncurses_colors()
{
	return (use_default_colors() == ERR ||
	        start_color() == ERR ||
	        init_color_pairs()) ? -1 : 0;
}

static int start_color_if_supported()
{
	if ((COLORED_OUTPUT = has_colors()))
		return start_ncurses_colors();
	else 
		return 0;
}

static inline int init_local_setup(WINDOW *wp)
{
	return (keypad(wp, TRUE) == ERR || 
		wattrset(wp, A_BOLD) == ERR) ? -1 : 0;
}

/*
 * Make the cursor invisible 
 */
static inline int invisibilize_cursor()
{
	return curs_set(0) == ERR ? -1 : 0;
}

int nc_init_setup()
{
	return (start_color_if_supported() || 
		cbreak() == ERR || noecho() == ERR || 
		invisibilize_cursor() || 
		init_local_setup(stdscr)) ? -1 : 0;
}

static int print_opening_message(WINDOW *wp)
{
	const char *message = "Ncurses disk analyzer --- Press ? for help";
	short cpair = COLORED_OUTPUT ? CYAN_PAIR : 0;

	return (mvwprintw(wp, 0, 0, "%s", message) == ERR || 
	        mvwchgat(wp, 0, 0, -1, A_REVERSE, cpair, NULL) == ERR) ? -1 : 0;
}

static off_t get_total_disk_usage(struct doubly_list *const head)
{
	struct doubly_list *current;
	off_t total;

	total = 0;

	for (current=head; current; current=current->next)
		if (current->data->file_data->fstatus)
			total += current->data->file_data->fstatus->st_size;
	return total;
}

static struct size_format proper_size_format(float size, char *unit)
{
	struct size_format retval;

	retval.size_format = size;
	retval.size_unit = unit;

	return retval;
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

static struct size_format get_proper_size_format(off_t bytes)
{
	const off_t gb = 1000000000;
	const off_t mb = 1000000;
	const off_t kb = 1000;

	if (bytes >= gb)
		return proper_size_format(bytes_to_gb(bytes), "GB");
	else if (bytes >= mb)
		return proper_size_format(bytes_to_mb(bytes), "MB");
	else if (bytes >= kb)
		return proper_size_format(bytes_to_kb(bytes), "KB");
	else
		return proper_size_format(bytes, "B");
}

static inline int summary_message(WINDOW *wp, 
				  struct doubly_list *const head,
				  const char *path, int y)
{
	struct size_format format;
	
	format = get_proper_size_format(get_total_disk_usage(head));

	return (mvwprintw(wp, y, 0, 
			  "Path: %s\t Total disk usage: %0.2f %s\t Items: ",
			  path, format.size_format, format.size_unit/*, 
			  get_total_path_items(path)*/) == ERR) ? -1 : 0;
}

static int print_summary_message(WINDOW *wp, 
				 struct doubly_list *const head, 
				 const char *current_path)
{
	short cpair = COLORED_OUTPUT ? CYAN_PAIR : 0;
	int y, x;

	getmaxyx(wp, y, x);

	return (summary_message(wp, head, current_path, y-1) ||
		mvwchgat(wp, y-1, 0, -1, A_REVERSE, cpair, NULL) == ERR) ? -1 : 0;
}

static int create_borders(WINDOW *wp)
{
	int mx, my;
	
	getmaxyx(wp, my, mx);

	return (mvwhline(wp, 1, 0, '-', mx) == ERR || 
		mvwhline(wp, my-2, 0, '-', mx) == ERR) ? -1 : 0; 
}

/*
 * This function is for the sake of readability
*/
static inline int restore_prev_entry_design(WINDOW *wp)
{
	short cpair = highligted_node->prev->data->curses_data->cpair;
	int y = highligted_node->prev->data->curses_data->y;
	
	return (mvwchgat(wp, y, 0, -1, A_BOLD, cpair, NULL) == ERR) ? -1 : 0;
}

/*
 * This function is for the sake of readability
 */
static inline int restore_next_entry_design(WINDOW *wp)
{
	short cpair = highligted_node->next->data->curses_data->cpair;
	int y = highligted_node->next->data->curses_data->y;
	
	return (mvwchgat(wp, y, 0, -1, A_BOLD, cpair, NULL) == ERR) ? -1 : 0;
}

static int restore_entry_design(WINDOW *wp, int key)
{
	if (key == KEY_DOWN)
		return restore_prev_entry_design(wp);
	else if (key == KEY_UP)
		return restore_next_entry_design(wp);
	else	
		return 0;
}

static int update_highlight_loc(WINDOW *wp, int key)
{
	int y = highligted_node->data->curses_data->y;

	return (mvwchgat(wp, y, 0, -1, A_REVERSE | A_BOLD, 0, NULL) == ERR ||
	        restore_entry_design(wp, key)) ? -1 : 0;
}

static inline int init_highlight(WINDOW *wp, struct doubly_list *const head)
{
	highligted_node = head;
	
	return update_highlight_loc(wp, '\0');
}

/*
 * The initial display for each window
 */
int nc_initial_display(WINDOW *wp, 
		       struct doubly_list *const head,
		       const char *current_path)
{
	return (print_opening_message(wp) || 
		create_borders(wp) || nc_display_fname(wp, head) || 
		print_summary_message(wp, head, current_path) ||
		init_highlight(wp, head) || wrefresh(wp) == ERR) ? -1 : 0;
}

static inline int del_and_null_win(WINDOW **wp)
{
	int retval;

	retval = delwin(*wp);
	*wp = NULL;

	return retval;
}

/*
 * Return a new window with some initial setup
 */
WINDOW *nc_newwin(int lines_num, int cols_num, int begin_y, int begin_x)
{
	WINDOW *wp;

	if ((wp = newwin(lines_num, cols_num, begin_y, begin_x)))
		if (init_local_setup(wp))
			del_and_null_win(&wp);
	return wp;
}

/*
 * Check if c is in the navigation keys, and return the value of the 
 * operation that this key does
 */
static inline int in_navigation_keys(int c)
{
	if (c == KEY_UP || c == 'k')
		return KEY_UP;
	else if (c == KEY_DOWN || c == 'j')
		return KEY_DOWN;
	else 
		return 0;
}

static int change_highlight_loc(WINDOW *wp, int key)
{
	if (key == KEY_DOWN && highligted_node->next)
		highligted_node = highligted_node->next;
	else if (key == KEY_UP && highligted_node->prev)
		highligted_node = highligted_node->prev;

	return (update_highlight_loc(wp, key) || wrefresh(wp) == ERR) ? -1 : 0;
}

static int perform_input_operations(WINDOW *wp, int c)
{
	int key;

	if ((key = in_navigation_keys(c))) {
		if (change_highlight_loc(wp, key))
			return -1;
	} else if ((c == 'q'))
		return 1;
	return 0;
}

int nc_man_input(WINDOW *wp)
{
	int c;

	while ((c = wgetch(wp)) != ERR)
		if (perform_input_operations(wp, c))
			return -1;
	return c;
}
