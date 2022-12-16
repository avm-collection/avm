#include <stdio.h>   /* printf, puts */
#include <stdlib.h>  /* exit, EXIT_SUCCESS, EXIT_FAILURE */
#include <string.h>  /* strcmp */
#include <stdbool.h> /* bool, true, false */

#include "config.h"
#include "platform.h"
#include "vm.h"

/*
 * 0.1.0: Basic stack operations
 * 0.2.0: Loops and logic
 * 0.3.0: cal and ret instructions
 * 0.3.1: Parameters improvements
 * 0.4.1: Add float and signed instructions, debug print instructions
 * 0.4.2: Fix platform and compiler detection
 * 1.0.0: Remove registers from instructions, from the VM and
 *        remove register manipulation instructions
 * 1.1.0: Improve swap and dup instructions
 * 1.1.1: Allocate the stack on the heap
 * 1.2.1: Add a call stack
 * 1.2.2: Only give a minor version warning if the exe minor version
 *        is greater than the avm minor version
 * 1.3.2: Improve vm messages, panic output
 * 1.4.2: Add a debugging mode
 * 1.4.3: Use readline when possible
 * 1.5.3: Add memory, memory operation instructions
 * 1.6.3: Add neg, not instructions
 * 1.7.3: Memory initialization from executable bytecode
 * 1.7.4: Fix r08 instruction
 *
 */

void usage(void);
void version(void);

void error(const char *p_fmt, ...);
void try(const char *p_flag);
