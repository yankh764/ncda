/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for managing the ncurses windows                      |
---------------------------------------------------------
*/

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "disk.h"
#include "general.h"
#include "curses_man.h"

#define EOL -1
#define NONE 0


/* Constant parameters */
const char _separator = '|';
const char _border = '-';
const attr_t _def_attrs = A_BOLD;
const int _borders_cpair = CYAN_PAIR;
const int _blank = 1;
const int _sep_blank = 2 * _blank;
const int _fsize_init_x = 0;
const int _mtime_init_x = 12;
const int _fname_init_x = 27;
const int _max_fsize_len = 8;
const int _max_mtime_len = 11;
const int _min_y = 2;

struct dtree *_highligted_node; 


static inline int print_separator(WINDOW *wp, int y, int x)
{
	return (mvwprintw(wp, y, x, "%c", _separator) == ERR) ? -1 : 0;
}

static inline int dye_text(WINDOW *wp, int y, int x, 
			   int len, attr_t add_attrs, short cpair)
{
	const attr_t attrs =  _def_attrs | add_attrs;

	return (mvwchgat(wp, y, x, len, attrs, cpair, NULL) == ERR) ? -1 : 0;
}

static inline int dye_val(WINDOW *wp, int y, int x, int len)
{
	const short cpair = YELLOW_PAIR;;

	return dye_text(wp, y, x, len, NONE, cpair);
}

static inline int dye_fsize(WINDOW *wp, int y)
{
	const int max_val_len = 5;

	return dye_val(wp, y, _fsize_init_x, max_val_len);
}

static int print_fsize(WINDOW *wp, int y, off_t bytes)
{
	struct size_format format;

	format = get_proper_size_format(bytes);

	if (mvwprintw(wp, y, _fsize_init_x, "%5.1f %s", format.val, format.unit) == ERR)
		return -1;
	if (COLORED_OUTPUT)
		if (dye_fsize(wp, y))
			return -1;
	return 0;
}

static inline int dye_mtime(WINDOW *wp, int y)
{
	const int max_val_len = 2;

	return dye_val(wp, y, _mtime_init_x, max_val_len);
}

static int print_mtime_buffer(WINDOW *wp, int y, char *buffer)
{
	int ret;

	ret = mvwprintw(wp, y, _mtime_init_x, "%s", buffer);
	free(buffer);

	return (ret == ERR) ? -1 : 0;
}

static int print_mtime(WINDOW *wp, int y, time_t mtime)
{
	char *buffer;

	if(!(buffer = get_mtime_str(mtime)))
		return -1;
	/* print_mtime_buffer() will take care of freeing the buffer */
	if (print_mtime_buffer(wp, y, buffer))
		return -1;
	if (COLORED_OUTPUT)
		if (dye_mtime(wp, y))
			return -1;
	return 0;
}

static inline int dye_fname(WINDOW *wp, int y, short cpair)
{
	return dye_text(wp, y, _fname_init_x, EOL, NONE, cpair);
}

static int print_fname(WINDOW *wp, int y, const char *name, char eos, short cpair)
{
	if (mvwprintw(wp, y, _fname_init_x, "%s%c", name, eos) == ERR)
		return -1;
	if (COLORED_OUTPUT)
		if (dye_fname(wp, y, cpair))
			return -1;
	return 0;
}

static int print_separators_only(WINDOW *wp, int y)
{
	int x1; 
	int x2;

	x1 = _fsize_init_x + _max_fsize_len + _sep_blank;
	x2 = _mtime_init_x + _max_mtime_len + _sep_blank;

	return (print_separator(wp, y, x1) || print_separator(wp, y, x2)) ? -1 : 0;
}

static inline int print_entry_size(WINDOW *wp, const struct dtree *node, int y)
{
	const off_t size = node->data->file->fsize;

	return print_fsize(wp, y, size);
}

static inline int print_entry_mtime(WINDOW *wp, const struct dtree *node, int y)
{
	const time_t mtime = node->data->file->fstatus->st_mtim.tv_sec;

	return print_mtime(wp, y, mtime);
}

static int print_entry_info(WINDOW *wp, const struct dtree *node, int y)
{
	return (print_entry_size(wp, node, y) || print_entry_mtime(wp, node, y)) ? -1 : 0;
}

static int display_entries_info(WINDOW *wp, const struct dtree *node)
{
	const short color_pair = node->data->curses->cpair;
	const char *const name = node->data->file->fname;
	const char eos = node->data->curses->eos;
	const int y = node->data->curses->y;
	
	if (!is_dot_entry(name)) {
		if (print_entry_info(wp, node, y))
			return -1;
	}
	if (print_fname(wp, y, name, eos, color_pair))
		return -1;
	else
		return print_separators_only(wp, y);
}

static int get_max_practical_y(WINDOW *wp)
{
	const int skipped_lines = 3;

	return (getmaxy(wp) - skipped_lines);
}

static bool is_between_page_borders(WINDOW *wp, int current_y)
{
	return (_min_y <= current_y) && (get_max_practical_y(wp) >= current_y);
}

int display_entries(WINDOW *wp, const struct dtree *ptr) 
{
	const struct dtree *current;
	int retval;

	retval = 0;

	for (current=ptr; current; current=current->next) {
		if (!is_between_page_borders(wp, current->data->curses->y))
			break;
		if ((retval = display_entries_info(wp, current)))
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
		init_pair(RED_PAIR,     COLOR_RED,     -1) == ERR) ? -1 : 0;
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

static inline int dye_bg(WINDOW *wp, int y, int x, int len, 
			 attr_t add_attrs, short cpair)
{
	const attr_t attrs = A_REVERSE | add_attrs;

	return (mvwchgat(wp, y, x, len, attrs, cpair, NULL) == ERR) ? -1 : 0;
}

static int display_opening_message(WINDOW *wp)
{
	const char *message = "Ncurses disk analyzer --- Press ? for help";
	const int begin_y = 0;
	const int begin_x = 0;
	short cpair;

	cpair = COLORED_OUTPUT ? _borders_cpair : DEFAULT_PAIR;

	if (mvwprintw(wp, begin_y, begin_x, "%s", message) == ERR)
		return -1;
	else
		return dye_bg(wp, begin_y, begin_x, EOL, NONE, cpair);
}

static int print_path_summary(WINDOW *wp, int y, int x, const char *path)
{
	return (mvwprintw(wp, y, x, "Path: %s", path) == ERR) ? -1 : 0;
}

static inline int print_usage(WINDOW *wp, int y, int x, 
			      const char *message, 
			      struct size_format fmt)
{
	return (mvwprintw(wp, y, x, "%s %0.2f %s", 
			  message, fmt.val, fmt.unit) == ERR) ? -1 : 0;
}

static int print_usage_summary(WINDOW *wp, int y, int max_x, const struct dtree *begin)
{
	const char *message = "Total Disk Usage:";
	struct size_format format;
	size_t len;

	format = get_proper_size_format(get_dtree_disk_usage(begin));
	/* The 1 is because I added another digit after the floating point */
	len = strlen(message) + _blank + (_max_fsize_len + 1);

	return print_usage(wp, y, max_x-len, message, format);
}

static int summary_message(WINDOW *wp, int y, int x,
			   const struct dtree *begin, 
			   const char *path)
{
	const int begin_x = 1;

	return (print_path_summary(wp, y, begin_x, path) || 
		print_usage_summary(wp, y, x, begin)) ? -1 : 0;
}

static int display_summary_message(WINDOW *wp, 
		   		   const struct dtree *begin, 
				   const char *current_path)
{
	const int begin_x = 0;
	int max_y, max_x;
	short cpair;

	cpair = COLORED_OUTPUT ? _borders_cpair : DEFAULT_PAIR;
	getmaxyx(wp, max_y, max_x);
	/* I decreased max_y and max_x because strangely they can't be displayed */
	if (summary_message(wp, --max_y, --max_x, begin, current_path))
		return -1;
	else 
		return dye_bg(wp, max_y, begin_x, EOL, _def_attrs, cpair);
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
			return -1;
	}
	return print_separators_only(wp, y);
}

static inline int print_borderline(WINDOW *wp, int y, int x, int len)
{
	return (mvwhline(wp, y, x, _border, len) == ERR) ? -1 : 0;
}

static int print_borders(WINDOW *wp)
{
	/* 2 = line that cant be displayed + summary message */
	const int skipped_lines = 2;
	const int begin_y = 1;
	const int begin_x = 0;
	int max_x, max_y;
	
	getmaxyx(wp, max_y, max_x);
	max_y -= skipped_lines;

	return (print_borderline(wp, begin_y, begin_x, max_x) || 
		print_borderline(wp, max_y, begin_x, max_x)) ? -1 : 0;
}

static inline int restore_entry_colors(WINDOW *wp, int y, short cpair)
{
	return (dye_fsize(wp, y) || 
		dye_mtime(wp, y) || 
		dye_fname(wp, y, cpair)) ? -1 : 0;
}

static inline int undye_bg(WINDOW *wp, int y, int x, int len)
{
	return (mvwchgat(wp, y, x, len, _def_attrs, DEFAULT_PAIR, NULL) == ERR) ? -1 : 0;
}

static int restore_prev_entry_design(WINDOW *wp)
{
	const short cpair = _highligted_node->prev->data->curses->cpair;
	const int y = _highligted_node->prev->data->curses->y;
	const int begin_x = 0;
	
	if (undye_bg(wp, y, begin_x, EOL))
		return -1;
	if (COLORED_OUTPUT)
		return restore_entry_colors(wp, y, cpair);
	else 
		return 0;
}

static int restore_next_entry_design(WINDOW *wp)
{
	const short cpair = _highligted_node->next->data->curses->cpair;
	const int y = _highligted_node->next->data->curses->y;
	const int begin_x = 0;
	
	if (undye_bg(wp, y, begin_x, EOL))
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
	else
		return restore_next_entry_design(wp);
}

/*
 * Manage highlight operation
 */
static int man_highlight_operation(WINDOW *wp, int key)
{
	const int y = _highligted_node->data->curses->y;
	const int begin_x = 0;

	if (dye_bg(wp, y, begin_x, EOL, _def_attrs, DEFAULT_PAIR))
		return -1;
	if (restore_entry_design(wp, key))
		return -1;
	else
		return (wrefresh(wp) == ERR) ? -1 : 0;
}

static int highlight_entry(WINDOW *wp, struct dtree *node)
{
	const int y = node->data->curses->y;
	const int begin_x = 0;

	if (dye_bg(wp, y, begin_x, EOL, _def_attrs, DEFAULT_PAIR))
		return -1;
	else
		return (wrefresh(wp) == ERR) ? -1 : 0;
}

static inline int init_highlight(WINDOW *wp, struct dtree *begin)
{
	_highligted_node = begin;
	
	return highlight_entry(wp, begin);
}

/*
 * The initial display for each window
 */
int nc_initial_display(WINDOW *wp, struct dtree *begin, const char *current_path)
{
	return (display_opening_message(wp) || print_borders(wp) ||
		display_labels(wp) || display_entries(wp, begin) || 
		display_summary_message(wp, begin, current_path) ||
		init_highlight(wp, begin)) ? -1 : 0;
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

static struct dtree *get_parent(struct dtree *ptr)
{
	struct dtree *current, *node; 

	node = NULL;

	for (current=ptr; current; current=current->prev)
		node = current;
	return node->parent;
}

/*
 * Returns the address of the node with the minumum y that can be displayed 
 */
static struct dtree *decrease_prev_y()
{
	struct dtree *current, *retval;

	retval = NULL;

	for (current=_highligted_node->prev; current; current=current->prev) {
		current->data->curses->y -= 1;
		
		if (current->data->curses->y == _min_y)
			retval = current;
	}
	return retval;
}

static inline void decrease_next_y()
{
	struct dtree *current;

	for (current=_highligted_node->next; current; current=current->next)
		current->data->curses->y -= 1;
}

/*
 * Returns the fisrt node then needs to be displayed on the screen
 */
static struct dtree *decrease_nodes_y()
{
	/* Decrease the currently highlighted node's y*/
	_highligted_node->data->curses->y -= 1;
	decrease_next_y();

	return decrease_prev_y();
}

static inline void increase_prev_y()
{
	struct dtree *current;

	for (current=_highligted_node->prev; current; current=current->prev)
		current->data->curses->y += 1;
}

static inline void increase_next_y()
{
	struct dtree *current;

	for (current=_highligted_node->next; current; current=current->next)
		current->data->curses->y += 1;
}

/*
 * Returns the fisrt node then needs to be displayed on the screen
 */
static struct dtree *increase_nodes_y()
{
	/* Increase the currently highlighted node's y*/
	_highligted_node->data->curses->y += 1;
	increase_prev_y();
	increase_next_y();

	return _highligted_node;;
}

static int clear_displayed_entries(WINDOW *wp)
{
	const char blank = ' ';
	const int begin_x = 0;
	int y, retval;
	
	retval = 0;

	for (y=_min_y; y<=get_max_practical_y(wp); y++)
		if ((retval = mvwhline(wp, y, begin_x , blank, getmaxx(wp))) == ERR)
			break;
	return retval;
}

/*
 * Get the entry with the minimum displayed y i.e 2
 */
static struct dtree *get_first_displayed_entry(struct dtree *ptr)
{
	struct dtree *current;

	for (current=ptr; current; current=current->prev)
		if (current->data->curses->y == _min_y)
			break;
	return current;
}

static inline int recreate_prev_display(WINDOW *wp, 
				 	const struct dtree *first_entry, 
				 	const char *path)
{
	return (display_opening_message(wp) || print_borders(wp) ||
		display_labels(wp) || display_entries(wp, first_entry) || 
		display_summary_message(wp, first_entry, path) ||
		highlight_entry(wp, _highligted_node)) ? -1 : 0;
}

static inline bool is_available_node(const struct dtree *node)
{
	return node ? 1 : 0;
}

static int _navigate_upward(WINDOW *wp)
{
	struct dtree *begin;
	
	begin = _highligted_node = _highligted_node->prev;

	if (!is_between_page_borders(wp, _highligted_node->data->curses->y)) {
		begin = increase_nodes_y();
		
		if (clear_displayed_entries(wp) || display_entries(wp, begin))
			return -1;
	}
	return man_highlight_operation(wp, KEY_UP);
}

static int navigate_upward(WINDOW *wp)
{
	int retval;

	if ((retval = is_available_node(_highligted_node->prev)))
		retval = _navigate_upward(wp);
	return retval;
}

static int _navigate_downward(WINDOW *wp)
{
	struct dtree *begin;
	
	begin = _highligted_node = _highligted_node->next;

	if (!is_between_page_borders(wp, _highligted_node->data->curses->y)) {
		begin = decrease_nodes_y();
		
		if (clear_displayed_entries(wp) || display_entries(wp, begin))
			return -1;
	}
	return man_highlight_operation(wp, KEY_DOWN);
}

static int navigate_downward(WINDOW *wp)
{
	int retval;

	if ((retval = is_available_node(_highligted_node->next)))
		retval = _navigate_downward(wp);
	return retval;
}

static int _navigate_outward(WINDOW *wp)
{
	struct dtree *begin;
	char *path;
	int retval;

	begin = _highligted_node = get_parent(_highligted_node);

	if (begin->data->curses->y != _min_y)
		begin = get_first_displayed_entry(_highligted_node);
	if (werase(wp) == ERR)
		return -1;
	if (!(path = extract_dir_path(begin->data->file->fpath)))
		return -1;
		
	retval = recreate_prev_display(wp, begin, path);
	free(path);

	return retval;
}

static int navigate_outward(WINDOW *wp)
{
	int retval;

	if ((retval = is_available_node(get_parent(_highligted_node))))
		retval = _navigate_outward(wp);
	return retval;
}

static int _navigate_inward(WINDOW *wp)
{
	char *path;
	int retval;

	_highligted_node = _highligted_node->child;

	if (werase(wp) == ERR)
		return -1; 
	if (!(path = extract_dir_path(_highligted_node->data->file->fpath)))
		return -1;

	retval = nc_initial_display(wp, _highligted_node, path);
	free(path);

	return retval;
}

static int navigate_inward(WINDOW *wp)
{
	int retval;

	if ((retval = is_available_node(_highligted_node->child)))
		retval = _navigate_inward(wp);
	return retval;
}

static int perform_navigation(WINDOW *wp, int c)
{
	if (c == KEY_UP || c == 'k')
		return navigate_upward(wp);
	else if (c == KEY_DOWN || c == 'j')
		return navigate_downward(wp);
	else if (c == KEY_ENTER || c == KEY_RIGHT || c == 'l' || c == '\n')
		return navigate_inward(wp);
	else if (c == KEY_BACKSPACE || c == KEY_LEFT || c == 'h' || c == '\b')
		return navigate_outward(wp);
	else 
		return 0;
}

static int perform_input_operations(WINDOW *wp, int c)
{
	if (c == 'c') {
		return 0;//rm_entry(_highligted_node->data->file);
	} else if (c == 'q'){
		return 1;
	} else {
		return perform_navigation(wp, c);
	}
}

int nc_man_input(WINDOW *wp)
{
	int c;

	while ((c = wgetch(wp)) != ERR) {
		if (perform_input_operations(wp, c))
			return -1;
	}
	return 0;
}

