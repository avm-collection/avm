#include <stdio.h>  /* printf, puts */
#include <stdlib.h> /* exit, EXIT_SUCCESS, EXIT_FAILURE, size_t */
#include <string.h> /* strcmp */
#include <stdarg.h> /* va_list, va_start, va_end, vsnprintf */

#include "utils.h"
#include "vm.h"

// 0.1.0: Basic stack operations
// 0.2.0: Loops and logic
// 0.3.0: cal and ret instructions

#define VERSION_MAJOR 0
#define VERSION_MINOR 3
#define VERSION_PATCH 0

#define APP_NAME "avm"

void usage(void);
void version(void);

void error(const char *p_fmt, ...);
void try(const char *p_flag);
