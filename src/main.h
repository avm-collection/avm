#include <stdio.h>  /* printf, puts */
#include <stdlib.h> /* exit, EXIT_SUCCESS, size_t */
#include <string.h> /* strcmp */

#include "utils.h"
#include "vm.h"

#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

#define APP_NAME "avm"

void usage(void);
void version(void);
