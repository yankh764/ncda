#ifndef _CURSES_H
#define _CURSES_H

#include <ncurses.h>
#include "structs.h"

extern bool COLORED_OUTPUT;

int nc_init_setup();
WINDOW *nc_newwin(int, int, int, int);
void nc_get_cdata_fields(struct entries_dlist *);
int nc_initial_display(WINDOW *, const struct entries_dlist *, const char *);
int nc_man_input(WINDOW *);

#endif
