#ifndef MAIN_H__HEADER_GUARD__
#define MAIN_H__HEADER_GUARD__

#include <stdio.h>   /* printf, puts */
#include <stdlib.h>  /* exit, EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>  /* strcmp */
#include <stdbool.h> /* bool, true, false */

#include "avm/vm.h"
#include "loader.h"

void usage(void);
void version(void);

void error(const char *p_fmt, ...);
void try(const char *p_flag);

#endif
