#include <stdio.h>   /* printf, puts */
#include <stdlib.h>  /* exit, EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>  /* strcmp */
#include <stdbool.h> /* bool, true, false */

#include "config.h"
#include "platform.h"
#include "vm.h"

/*
 * 0.1.0:  Basic stack operations
 * 0.2.0:  Loops and logic
 * 0.3.0:  cal and ret instructions
 * 0.3.1:  Parameters improvements
 * 0.4.1:  Add float and signed instructions, debug print instructions
 * 0.4.2:  Fix platform and compiler detection
 * 1.4.2:  Remove registers from instructions, from the VM and
 *         remove register manipulation instructions
 * 1.5.2:  Improve swap and dup instructions
 * 1.5.3:  Allocate the stack on the heap
 * 1.6.3:  Add a call stack
 * 1.6.4:  Only give a minor version warning if the exe minor version
 *         is greater than the avm minor version
 * 1.7.4:  Improve vm messages, panic output
 * 1.8.4:  Add a debugging mode
 * 1.8.5:  Use readline when possible
 * 1.9.5:  Add memory, memory operation instructions
 * 1.10.5: Add neg, not instructions
 *
 */

void usage(void);
void version(void);

void error(const char *p_fmt, ...);
void try(const char *p_flag);
