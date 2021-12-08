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
#include "ncda.h"
#include "disk_man.h"
#include "curses.h"

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
	RED_PAIR = 6,
	DEFAULT_PAIR = 7
};

const struct doubly_list *highligted_node; 
const int def_attrs = A_BOLD;
const int fname_x = 9;



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

static inline short proper_color_pair(mode_t file_mode)
{
	if (COLORED_OUTPUT) 
		return get_color_pair_num(file_mode);
	else 
		return 0;
}

/*
 * Get the proper end of a string. If the file is a directory the end of
 * the string will be slash ('/'), else the end of a string will be blank
 * character (' ').
 */
static inline char proper_eos(mode_t file_mode)
{
	return S_ISDIR(file_mode) ? '/' : ' ';
}

static inline void insert_cdata_fields(struct entry_data *ptr, int i)
{
	struct stat *statbuf = ptr->file_data->fstatus;

	ptr->curses_data->cpair = proper_color_pair(statbuf->st_mode);
	/* The 2 = an empty line + a border line */
	ptr->curses_data->y = i + 2;
	ptr->curses_data->eos = proper_eos(statbuf->st_mode);
}

void nc_get_cdata_fields(struct doubly_list *head)
{
	struct doubly_list *current;
	unsigned int i;
	
	for (current=head, i=0; current; current=current->next, i++)
		insert_cdata_fields(current->data, i);
}

static inline int dye_fsize(WINDOW *wp, int y, int x, int len)
{
	const short cpair = RED_PAIR;

	return (mvwchgat(wp, y, x, len, def_attrs, cpair, NULL) == ERR) ? -1 : 0;
}

static inline int print_size(WINDOW *wp, int y, int x, float size)
{
	return (mvwprintw(wp, y, x, "%.1f", size) == ERR) ? -1 : 0;
}

static inline int print_size_unit(WINDOW *wp, int y, int x, const char *unit)
{
	/* 1 is equal to the space lenght */
	return (mvwprintw(wp, y, x, "%s", unit) == ERR) ? -1 : 0;
}

static int print_fsize(WINDOW *wp, int y, int x, off_t bytes)
{
	const int max_size_len = 5;
	struct size_format format;
	int curx;

	format = get_proper_size_format(bytes);

	if (print_size(wp, y, x, format.size) || 
	    print_size_unit(wp, y, x+max_size_len+1, format.unit))
		return -1;
	if ((curx = getcurx(wp)) == ERR)
		return -1;
	if (COLORED_OUTPUT)
		if (dye_fsize(wp, y, x, curx-x))
			return -1;
	return curx;
}

static inline int dye_fname(WINDOW *wp, int y, int len, short cpair)
{
	return (mvwchgat(wp, y, fname_x, len, 
			 def_attrs, cpair, NULL) == ERR) ? -1 : 0;
}

static int print_fname(WINDOW *wp, int y, const char *name, char eos, short cpair)
{
	int curx;

	if (mvwprintw(wp, y, fname_x, "%s%c", name, eos) == ERR)
		return -1;
	if ((curx = getcurx(wp)) == ERR)
		return -1;
	if (cpair)
		if (dye_fname(wp, y, curx-fname_x, cpair))
			return -1; 
	return curx;
}

static int print_st_mtim(WINDOW *wp, int y, int x, const struct timespec tm)
{
	int curx;
}

static inline int display_entries_info(WINDOW *wp,
				       const struct doubly_list *node)
{
	const struct timespec tm = node->data->file_data->fstatus->st_mtime;
	const off_t fsize = node->data->file_data->fstatus->st_size;
	const short color_pair = node->data->curses_data->cpair;
	const char *name = node->data->file_data->fname;
	const char eos = node->data->curses_data->eos;
	const int y = node->data->curses_data->y;
	const int begin_x = 0;
	int curx;

	if (!is_dot_entry(name)) {
		if ((curx = print_fsize(wp, y, begin_x, fsize) == -1))
			return -1;
		if ((curx = print_st_mtim(wp, y, curx+1, tm)) == -1)
			return -1;
	}
	if ((curx = print_fname(wp, y, name, eos, color_pair) == -1))
		return -1;
	return 0;
}

int nc_display_entries(WINDOW *wp, const struct doubly_list *head) 
{
	const struct doubly_list *current;
	int retval;

	retval = 0;

	for (current=head; current; current=current->next)
		if ((retval = display_entries_info(wp, current)) == -1)
			break;
	return retval;
}

static int init_color_pairs()
{
	return (init_pair(BLUE_PAIR,    COLOR_BLUE,    -1) == ERR ||
	        init_pair(GREEN_PAIR,   COLOR_GREEN,   -1) == ERR ||
	        init_pair(YELLOW_PAIR,  COLOR_YELLOW,  -1) == ERR ||
	        init_pair(CYAN_PAIR,    COLOR_CYAN,    -1) == ERR ||
	        init_pair(MAGENTA_PAIR, COLOR_MAGENTA, -1) == ERR ||
		init_pair(RED_PAIR,     COLOR_RED,     -1) == ERR ||
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

static int init_local_setup(WINDOW *wp)
{
	return (keypad(wp, TRUE) == ERR || 
		wattrset(wp, def_attrs) == ERR) ? -1 : 0;
}

/*
 * Make the cursor invisible 
 */
static int invisibilize_cursor()
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

static int print_path(WINDOW *wp, const char *path, int y)
{
	return (mvwprintw(wp, y, 0, " Path: %s", path) == ERR) ? -1 : 0;
}

static int print_usage(WINDOW *wp, const struct doubly_list *head, int y, int x)
{
	const char *message = "Total Disk Usage: ";
	struct size_format format;
	size_t len;

	/* 6 is the format.size, 1 is the space, 2 is format.unit */
	len = strlen(message) + 6 + 1 + 2;
	format = get_proper_size_format(get_total_disk_usage(head));

	return (mvwprintw(wp, y, x-len, "%s%0.2f %s", 
		message, format.size, format.unit) == ERR) ? -1 : 0;
}

static int summary_message(WINDOW *wp, 
			   const struct doubly_list *head,
			   const char *path, int y, int x)
{
	return (print_path(wp, path, y) || print_usage(wp, head, y, x));
}

static int print_summary_message(WINDOW *wp, 
				 const struct doubly_list *head, 
				 const char *current_path)
{
	short cpair = COLORED_OUTPUT ? CYAN_PAIR : 0;
	int y, x;

	getmaxyx(wp, y, x);
	
	return (summary_message(wp, head, current_path, --y, --x) ||
		mvwchgat(wp, y, 0, -1, 
			 A_REVERSE | A_BOLD, 
			 cpair, NULL) == ERR) ? -1 : 0;
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
	const short cpair = highligted_node->prev->data->curses_data->cpair;
	const char *name = highligted_node->prev->data->file_data->fname;
	const int y = highligted_node->prev->data->curses_data->y;
	const int len = strlen(name) + 1;
	
	return (mvwchgat(wp, y, 0, -1, A_BOLD, 0, NULL) == ERR || 
		dye_fname(wp, y, len, cpair)) ? -1 : 0;
}

/*
 * This function is for the sake of readability
 */
static inline int restore_next_entry_design(WINDOW *wp)
{
	const short cpair = highligted_node->next->data->curses_data->cpair;
	const char *name = highligted_node->next->data->file_data->fname;
	const int y = highligted_node->next->data->curses_data->y;
	const int len = strlen(name) + 1;
	
	return (mvwchgat(wp, y, 0, -1, A_BOLD, 0, NULL) == ERR || 
		dye_fname(wp, y, len, cpair)) ? -1 : 0;
}

static inline int restore_entry_design(WINDOW *wp, int key)
{
	if (key == KEY_DOWN)
		return restore_prev_entry_design(wp);
	else if (key == KEY_UP)
		return restore_next_entry_design(wp);
	else	
		return 0;
}

static inline int update_highlight_loc(WINDOW *wp, int key)
{
	int y = highligted_node->data->curses_data->y;

	return (mvwchgat(wp, y, 0, -1, A_REVERSE | A_BOLD, 0, NULL) == ERR ||
	        restore_entry_design(wp, key)) ? -1 : 0;
}

static int init_highlight(WINDOW *wp, const struct doubly_list *head)
{
	highligted_node = head;
	
	return update_highlight_loc(wp, '\0');
}

/*
 * The initial display for each window
 */
int nc_initial_display(WINDOW *wp, 
		       const struct doubly_list *head,
		       const char *current_path)
{
	return (print_opening_message(wp) || 
		create_borders(wp) || nc_display_entries(wp, head) || 
		print_summary_message(wp, head, current_path) ||
		init_highlight(wp, head) || wrefresh(wp) == ERR) ? -1 : 0;
}

static int del_and_null_win(WINDOW **wp)
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

static inline int change_highlight_loc(WINDOW *wp, int key)
{
	if (key == KEY_DOWN && highligted_node->next)
		highligted_node = highligted_node->next;
	else if (key == KEY_UP && highligted_node->prev)
		highligted_node = highligted_node->prev;

	return (update_highlight_loc(wp, key) || wrefresh(wp) == ERR) ? -1 : 0;
}

static inline int perform_input_operations(WINDOW *wp, int c)
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
