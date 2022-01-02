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

struct doubly_list *_highligted_node; 


/* Constant parameters */
const int _def_attrs = A_BOLD;
const int _borders_cpair = CYAN_PAIR;
const char _separator = '|';
const int _blank = 1;
const int _sep_blank = 2 * _blank;
const int _fsize_init_x = 0;
const int _mtime_init_x = 12;
const int _fname_init_x = 27;
const int _max_print_fsize = 8;
const int _max_print_mtime = 11;



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
	struct stat *const statbuf = ptr->file_data->fstatus;
	/* The 2 = an empty line + labels line */
	const int skipped_lines = 2;

	ptr->curses_data->cpair = proper_color_pair(statbuf->st_mode);
	ptr->curses_data->y = i + skipped_lines;
	ptr->curses_data->eos = proper_eos(statbuf->st_mode);
}

void nc_get_cdata_fields(struct doubly_list *head)
{
	struct doubly_list *current;
	unsigned int i;
	
	for (current=head, i=0; current; current=current->next, i++)
		insert_cdata_fields(current->data, i);
}

static inline int print_separator(WINDOW *wp, int y, int x)
{
	return (mvwprintw(wp, y, x, "%c", _separator) == ERR) ? -1 : 0;
}

static inline int dye_val(WINDOW *wp, int y, int x, int len)
{
	const short cpair = YELLOW_PAIR;;

	return (mvwchgat(wp, y, x, len, _def_attrs, cpair, NULL) == ERR) ? -1 : 0;
}

static inline int dye_fsize(WINDOW *wp, int y)
{
	const int max_val_len = 5;

	return dye_val(wp, y, _fsize_init_x, max_val_len);
}

static int print_fsize(WINDOW *wp, int y, off_t bytes)
{
	struct size_format format;
	int sep_x;

	format = get_proper_size_format(bytes);

	if (mvwprintw(wp, y, _fsize_init_x, "%5.1f %s", 
		      format.size, format.unit) == ERR)
		return -1;
	if (COLORED_OUTPUT)
		if (dye_fsize(wp, y))
			return -1;
	sep_x = _fsize_init_x + _max_print_fsize + _sep_blank;
	
	return print_separator(wp, y, sep_x);
}

static inline int dye_mtime(WINDOW *wp, int y)
{
	const int max_val_len = 1;

	return dye_val(wp, y, _mtime_init_x, max_val_len);
}

static int print_mtime(WINDOW *wp, int y, time_t mtime)
{
	char *buffer;
	int sep_x;

	if(!(buffer = get_mtime_str(mtime)))
		return -1;
	if (mvwprintw(wp, y, _mtime_init_x, "%s", buffer) == ERR)
		goto err_free_buf;
	if (COLORED_OUTPUT)
		if (dye_mtime(wp, y))
			goto err_free_buf;
	free(buffer);
	sep_x = _mtime_init_x + _max_print_mtime + _sep_blank;
	
	return print_separator(wp, y, sep_x);

err_free_buf:
	free(buffer);
	return -1;
}

static inline int dye_fname(WINDOW *wp, int y, short cpair)
{
	/* End of line */
	const int eol = -1;

	return (mvwchgat(wp, y, _fname_init_x, eol, 
			 _def_attrs, cpair, NULL) == ERR) ? -1 : 0;
}

static int print_fname(WINDOW *wp, int y, const char *name, char eos, short cpair)
{
	if (mvwprintw(wp, y, _fname_init_x, "%s%c", name, eos) == ERR)
		return -1;
	if (cpair)
		if (dye_fname(wp, y, cpair))
			return -1;
	return 0;
}

static int print_separators_only(WINDOW *wp, int y)
{
	int x1; 
	int x2;

	x1 = _fsize_init_x + _max_print_fsize + _sep_blank;
	x2 = _mtime_init_x + _max_print_mtime + _sep_blank;

	return (print_separator(wp, y, x1) || 
		print_separator(wp, y, x2)) ? -1 : 0;
}

static inline int display_entries_info(WINDOW *wp,
				       const struct doubly_list *node)
{
	const time_t mtime = node->data->file_data->fstatus->st_mtim.tv_sec;
	const off_t fsize = node->data->file_data->fstatus->st_size;
	const short color_pair = node->data->curses_data->cpair;
	const char *const name = node->data->file_data->fname;
	const char eos = node->data->curses_data->eos;
	const int y = node->data->curses_data->y;
	
	if (is_dot_entry(name)) {
		if (print_separators_only(wp, y))
			return -1;
	} else {
		if (print_fsize(wp, y, fsize))
			return -1;
		if (print_mtime(wp, y, mtime))
			return -1;
	}
	return print_fname(wp, y, name, eos, color_pair);
}

static inline bool is_between_page_borders(WINDOW *wp, int current_y)
{
	/* 3 = line that cant be displayed + summary message + border line */
	const int skipped_lines = 3;
	const int max_y = getmaxy(wp) - skipped_lines;
	const int min_y = 2;

	return (min_y <= current_y) && (max_y >= current_y);
}

int display_entries(WINDOW *wp, const struct doubly_list *ptr) 
{
	const struct doubly_list *current;
	int retval;

	retval = 0;

	for (current=ptr; current; current=current->next) {
		if (!is_between_page_borders(wp, current->data->curses_data->y))
			break;
		if ((retval = display_entries_info(wp, current)) == -1)
			break;
	}
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
		wattrset(wp, _def_attrs) == ERR) ? -1 : 0;
}

/*
 * Make the cursor invisible 
 */
static int invisibilize_cursor()
{
	return (curs_set(0) == ERR) ? -1 : 0;
}

int nc_init_setup()
{
	return (start_color_if_supported() || 
		cbreak() == ERR || noecho() == ERR || 
		invisibilize_cursor() || 
		init_local_setup(stdscr)) ? -1 : 0;
}

static int display_opening_message(WINDOW *wp)
{
	const char *message = "Ncurses disk analyzer --- Press ? for help";
	const int begin_y = 0;
	const int begin_x = 0;
	const int eol = -1;
	short cpair;

	cpair = COLORED_OUTPUT ? _borders_cpair : 0;

	if (mvwprintw(wp, begin_y, begin_x, "%s", message) == ERR)
		return -1;
	else
		return (mvwchgat(wp, begin_y, begin_x, eol,
				 A_REVERSE, cpair, NULL) == ERR) ? -1 : 0;
}

static int print_path_summary(WINDOW *wp, int y, int x, const char *path)
{
	return (mvwprintw(wp, y, x, "Path: %s", path) == ERR) ? -1 : 0;
}

static int print_usage_summary(WINDOW *wp, int y, int max_x,
			       const struct doubly_list *head)
{
	const char *message = "Total Disk Usage:";
	struct size_format format;
	size_t len;

	format = get_proper_size_format(get_total_disk_usage(head));
	/* The 1 is because I added another digit after the floating point */
	len = strlen(message) + _blank + _max_print_fsize + 1;

	return (mvwprintw(wp, y, max_x-len, "%s %0.2f %s", 
			  message, format.size, format.unit) == ERR) ? -1 : 0;
}

static int summary_message(WINDOW *wp, int y, int x,
			   const struct doubly_list *head, const char *path)
{
	const int begin_x = 1;

	return (print_path_summary(wp, y, begin_x, path) || 
		print_usage_summary(wp, y, x, head)) ? -1 : 0;
}

static int display_summary_message(WINDOW *wp, 
		   		   const struct doubly_list *head, 
				   const char *current_path)
{
	const int attrs = A_REVERSE | A_BOLD;
	const int begin_x = 0;
	const int eol = -1;
	int max_y, max_x;
	short cpair;

	cpair = COLORED_OUTPUT ? _borders_cpair : 0;
	getmaxyx(wp, max_y, max_x);

	if (summary_message(wp, --max_y, --max_x, head, current_path))
		return -1;
	else 
		return (mvwchgat(wp, max_y, begin_x, 
				 eol, attrs, cpair, NULL) == ERR) ? -1 : 0;
}

static int print_lables(WINDOW *wp, int y)
{
	const int begin_x = 3;

	return (mvwprintw(wp, y, begin_x, "Size") == ERR ||
		mvwprintw(wp, y, _mtime_init_x, "Modification") == ERR ||
		mvwprintw(wp, y, _fname_init_x, "Entry's name") == ERR) ? -1 : 0;
}

static int print_labels_colorful(WINDOW *wp, int y)
{
	const int cpair = RED_PAIR;

	return (wattron(wp, COLOR_PAIR(cpair)) == ERR ||
		print_lables(wp, y) ||
		wattroff(wp, COLOR_PAIR(cpair)) == ERR) ? -1 : 0;
}

static int display_labels(WINDOW *wp)
{
	const int y = 1;

	if (COLORED_OUTPUT) {
		if (print_labels_colorful(wp, y))
			return -1;
	} else {
		if (print_lables(wp, y))
			return 1;
	}
	return print_separators_only(wp, y);
}

static int create_borders(WINDOW *wp)
{
	const char border = '-';
	const int begin_y = 1;
	const int begin_x = 0;
	int max_x, max_y;
	
	getmaxyx(wp, max_y, max_x);
	/* 2 = line that cant be displayed + summary message */
	max_y -= 2;

	return (mvwhline(wp, begin_y, begin_x, border, max_x) == ERR || 
		mvwhline(wp, max_y, begin_x, border, max_x) == ERR) ? -1 : 0;
}

static inline int restore_entry_colors(WINDOW *wp, int y, short cpair)
{
	return (dye_fsize(wp, y) || 
		dye_mtime(wp, y) || 
		dye_fname(wp, y, cpair)) ? -1 : 0;
}

static inline int restore_prev_entry_design(WINDOW *wp)
{
	const short cpair = _highligted_node->prev->data->curses_data->cpair;
	const int y = _highligted_node->prev->data->curses_data->y;
	const int begin_x = 0;
	const int eol = -1;
	
	if (mvwchgat(wp, y, begin_x, eol, _def_attrs, 0, NULL) == ERR)
		return -1;
	if (COLORED_OUTPUT)
		return restore_entry_colors(wp, y, cpair);
	else 
		return 0;
}

static inline int restore_next_entry_design(WINDOW *wp)
{
	const short cpair = _highligted_node->next->data->curses_data->cpair;
	const int y = _highligted_node->next->data->curses_data->y;
	const int begin_x = 0;
	const int eol = -1;
	
	if (mvwchgat(wp, y, begin_x, eol, _def_attrs, 0, NULL) == ERR)
		return -1;
	if (COLORED_OUTPUT)
		return restore_entry_colors(wp, y, cpair);
	else 
		return 0;
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
	const int y = _highligted_node->data->curses_data->y;
	const int attrs = _def_attrs | A_REVERSE;
	const int begin_x = 0;
	const int eol = -1;

	if (mvwchgat(wp, y, begin_x, eol, attrs, 0, NULL) == ERR)
		return -1;
	if (restore_entry_design(wp, key))
		return -1;
	else
		return (wrefresh(wp) == ERR) ? -1 : 0;
}

static int init_highlight(WINDOW *wp, const struct doubly_list *head)
{
	const char null = '\0';

	_highligted_node = (struct doubly_list *) head;
	
	return update_highlight_loc(wp, null);
}

/*
 * The initial display for each window
 */
int nc_initial_display(WINDOW *wp, 
		       const struct doubly_list *head,
		       const char *current_path)
{
	return (display_opening_message(wp) || create_borders(wp) ||
		display_labels(wp) || display_entries(wp, head) || 
		display_summary_message(wp, head, current_path) ||
		init_highlight(wp, head)) ? -1 : 0;
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

static inline struct doubly_list *change_highlighted_node(int key)
{
	struct doubly_list *retval;

	if (key == KEY_DOWN && _highligted_node->next)
		retval =_highligted_node = _highligted_node->next;
	else if (key == KEY_UP && _highligted_node->prev)
		retval = _highligted_node = _highligted_node->prev;
	else 
		retval = NULL;
	
	return retval;
}

static inline struct doubly_list *decrease_prev_y()
{
	const int min_y = 2;
	struct doubly_list *current, *retval;

	for (current=_highligted_node->prev; current; current=current->prev) {
		current->data->curses_data->y -= 1;
		
		if (current->data->curses_data->y == min_y)
			retval = current;
	}
	return retval;
}

static inline void decrease_next_y()
{
	struct doubly_list *current;

	for (current=_highligted_node->next; current; current=current->next)
		current->data->curses_data->y -= 1;
}

static struct doubly_list *decrease_nodes_y()
{
	_highligted_node->data->curses_data->y -= 1;
	decrease_next_y();

	return decrease_prev_y();
}

static inline void increase_prev_y()
{
	struct doubly_list *current;

	for (current=_highligted_node->prev; current; current=current->prev)
		current->data->curses_data->y += 1;
}

static inline void increase_next_y()
{
	struct doubly_list *current;

	for (current=_highligted_node->next; current; current=current->next)
		current->data->curses_data->y += 1;
}

static struct doubly_list *increase_nodes_y()
{
	_highligted_node->data->curses_data->y += 1;
	increase_prev_y();
	increase_next_y();

	return _highligted_node;
}

static inline struct doubly_list *correct_nodes_y(int key)
{
	if (key == KEY_DOWN)
		return decrease_nodes_y();
	else
		return increase_nodes_y();
}

static int clear_screen(WINDOW *wp)
{
	int begin_y, max_y, max_x;
	
	getmaxyx(wp, max_y, max_x);
	max_y -= 2;

	for (begin_y=2; begin_y<max_y; begin_y++)
		mvwhline(wp, begin_y, 0, ' ', max_x);
	return 0;
}

static int manage_navigation_input(WINDOW *wp, int key)
{
	struct doubly_list *beginning;

	if (!change_highlighted_node(key))
		return 0;
	if (!is_between_page_borders(wp, _highligted_node->data->curses_data->y)) {
		beginning = correct_nodes_y(key);
		/* Dont forget cchecking error */
		clear_screen(wp);	
		display_entries(wp, beginning);
	}
	return update_highlight_loc(wp, key);
}

static inline int perform_input_operations(WINDOW *wp, int c)
{
	int key;

	if ((key = in_navigation_keys(c))) {
		if (manage_navigation_input(wp, key))
			return -1;
	}
	return 0;
}

int nc_man_input(WINDOW *wp)
{
	int c;

	while ((c = wgetch(wp)) != ERR) {
		switch (c) {
		case 'q':
			return 0;
		default:
			if (perform_input_operations(wp, c))
				return -1;
			break;
		}
	}
	return c;
}
