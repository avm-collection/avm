#include "utils.h"

void fatal(const char *p_fmt, ...) {
	char    msg[1024];
	va_list args;

	va_start(args, p_fmt);
	vsnprintf(msg, sizeof(msg), p_fmt, args);
	va_end(args);

	fprintf(stderr, "Fatal: %s\n", msg);

	exit(EXIT_FAILURE);
}
