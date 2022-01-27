#ifndef _CURSES_MAN_H
#define _CURSES_MAN_H

#include <ncurses.h>
#include "structs.h"


extern bool COLORED_OUTPUT;

enum COLOR_PAIRS_NUM {
	BLUE_PAIR =  1,
	GREEN_PAIR = 2, 
	YELLOW_PAIR = 3,
	CYAN_PAIR = 4,
	MAGENTA_PAIR = 5,
	RED_PAIR = 6,
	DEFAULT_PAIR = 7
};

int nc_init_setup();
void nc_insert_cdata_fields(struct dtree *);
int nc_initial_display(WINDOW *, const struct dtree *, const char *);
int nc_man_input(WINDOW *);

#endif
