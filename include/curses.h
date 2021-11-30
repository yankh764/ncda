#ifndef _CURSES_H
#define _CURSES_H

#include <ncurses.h>
#include "structs.h"

extern bool COLORED_OUTPUT;

int nc_display_fname(WINDOW *, const struct doubly_list *);
int nc_init_setup();
WINDOW *nc_newwin(int, int, int, int);
void nc_get_cdata_fields(struct doubly_list *);
int nc_initial_display(WINDOW *, const struct doubly_list *, const char *);
int nc_man_input(WINDOW *);

#endif
