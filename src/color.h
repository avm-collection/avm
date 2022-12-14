#ifndef COLOR_H__HEADER_GUARD__
#define COLOR_H__HEADER_GUARD__

#include <stdio.h> /* fputs, FILE */

#include "platform.h"

enum color {
	COLOR_DEFAULT = 0,

	COLOR_BLACK,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_MAGENTA,
	COLOR_CYAN,
	COLOR_WHITE,

	COLOR_GREY,
	COLOR_BRIGHT_RED,
	COLOR_BRIGHT_GREEN,
	COLOR_BRIGHT_YELLOW,
	COLOR_BRIGHT_BLUE,
	COLOR_BRIGHT_MAGENTA,
	COLOR_BRIGHT_CYAN,
	COLOR_BRIGHT_WHITE,
};

void set_fg_color(enum color p_color, FILE *p_file);
void set_bg_color(enum color p_color, FILE *p_file);

#endif
