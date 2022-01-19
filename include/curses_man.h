#ifndef _CURSES_MAN_H
#define _CURSES_MAN_H

#include <ncurses.h>
#include "structs.h"

extern bool COLORED_OUTPUT;

int nc_init_setup();
void nc_get_cdata_fields(struct dtree *);
int nc_initial_display(WINDOW *, const struct dtree *, const char *);
int nc_man_input(WINDOW *);

#endif
