#ifndef UTILS_H__HEADER_GUARD__
#define UTILS_H__HEADER_GUARD__

#include <stdio.h>  /* stderr, fprintf */
#include <stdlib.h> /* exit, EXIT_FAILURE */
#include <stdarg.h> /* va_list, va_start, va_end, vsnprintf */
#include <assert.h> /* assert */

#define UNREACHABLE() fatal("%s:%i: Unreachable", __FILE__, __LINE__)
#define TODO(P_MSG)   fatal("%s:%i: TODO: %s", __FILE__, __LINE__, P_MSG)

#define SILENCE_RETURN_WARNING() \
	fprintf(stderr, "%s:%i: Reached unreachable return warning silencer\n", __FILE__, __LINE__); \
	exit(EXIT_FAILURE)

void fatal(const char *p_fmt, ...);
void warning(const char *p_fmt, ...);

#endif
