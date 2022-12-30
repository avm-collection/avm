#ifndef UTILS_H__HEADER_GUARD__
#define UTILS_H__HEADER_GUARD__

#include <stdio.h>  /* stderr, fprintf */
#include <stdlib.h> /* exit, EXIT_FAILURE */
#include <stdarg.h> /* va_list, va_start, va_end, vsnprintf */
#include <string.h> /* strlen */
#include <ctype.h>  /* isspace */

#define FATAL(...) \
	fprintf(stderr, __VA_ARGS__); \
	exit(EXIT_FAILURE)

#define UNREACHABLE() FATAL("%s:%i: Unreachable", __FILE__, __LINE__)
#define TODO(P_MSG)   FATAL("%s:%i: TODO: %s", __FILE__, __LINE__, P_MSG)
#define UNUSED(P_X)   (void)P_X

#define ARRAY_SIZE(P_ARR) (sizeof(P_ARR) / sizeof(P_ARR[0]))

#define PARSE_FMT_INTO(P_FMT, P_NAME, P_SIZE) \
	char    P_NAME[P_SIZE]; \
	va_list args; \
	va_start(args, P_FMT); \
	vsnprintf(P_NAME, sizeof(P_NAME), P_FMT, args); \
	va_end(args);

char *strtrim(char *p_str);

#endif
