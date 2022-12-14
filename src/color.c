#include "color.h"

#ifdef PLATFORM_LINUX

const char *color_to_fg_escape_sequence[] = {
	[COLOR_DEFAULT] = "\x1b[0m",

	[COLOR_BLACK]   = "\x1b[30m",
	[COLOR_RED]     = "\x1b[31m",
	[COLOR_GREEN]   = "\x1b[32m",
	[COLOR_YELLOW]  = "\x1b[33m",
	[COLOR_BLUE]    = "\x1b[34m",
	[COLOR_MAGENTA] = "\x1b[35m",
	[COLOR_CYAN]    = "\x1b[36m",
	[COLOR_WHITE]   = "\x1b[37m",

	[COLOR_GREY]           = "\x1b[90m",
	[COLOR_BRIGHT_RED]     = "\x1b[91m",
	[COLOR_BRIGHT_GREEN]   = "\x1b[92m",
	[COLOR_BRIGHT_YELLOW]  = "\x1b[93m",
	[COLOR_BRIGHT_BLUE]    = "\x1b[94m",
	[COLOR_BRIGHT_MAGENTA] = "\x1b[95m",
	[COLOR_BRIGHT_CYAN]    = "\x1b[96m",
	[COLOR_BRIGHT_WHITE]   = "\x1b[97m",
};

const char *color_to_bg_escape_sequence[] = {
	[COLOR_DEFAULT] = "\x1b[0m",

	[COLOR_BLACK]   = "\x1b[40m",
	[COLOR_RED]     = "\x1b[41m",
	[COLOR_GREEN]   = "\x1b[42m",
	[COLOR_YELLOW]  = "\x1b[43m",
	[COLOR_BLUE]    = "\x1b[44m",
	[COLOR_MAGENTA] = "\x1b[45m",
	[COLOR_CYAN]    = "\x1b[46m",
	[COLOR_WHITE]   = "\x1b[47m",

	[COLOR_GREY]           = "\x1b[100m",
	[COLOR_BRIGHT_RED]     = "\x1b[101m",
	[COLOR_BRIGHT_GREEN]   = "\x1b[102m",
	[COLOR_BRIGHT_YELLOW]  = "\x1b[103m",
	[COLOR_BRIGHT_BLUE]    = "\x1b[104m",
	[COLOR_BRIGHT_MAGENTA] = "\x1b[105m",
	[COLOR_BRIGHT_CYAN]    = "\x1b[106m",
	[COLOR_BRIGHT_WHITE]   = "\x1b[107m",
};

#endif

void set_fg_color(enum color p_color, FILE *p_file) {
#ifdef PLATFORM_LINUX
	fputs(color_to_fg_escape_sequence[p_color], p_file);
#endif
}

void set_bg_color(enum color p_color, FILE *p_file) {
#ifdef PLATFORM_LINUX
	fputs(color_to_bg_escape_sequence[p_color], p_file);
#endif
}
