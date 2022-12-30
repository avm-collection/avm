#ifndef DEBUGGER_H__HEADER_GUARD__
#define DEBUGGER_H__HEADER_GUARD__

#include <string.h>  /* strcmp, strlen */
#include <stdlib.h>  /* free */

#include "avm/vm.h"

#ifdef PLATFORM_LINUX
#	define USES_READLINE
#endif

#ifdef USES_READLINE
#	include <readline/readline.h> /* readline, rl_set_signals */
#	include <readline/history.h>  /* add_history, using_history */

#	define RL_ESC_SEQ(P_SEQ) "\001"P_SEQ"\002"
#	define PROMPT RL_ESC_SEQ("\x1b[94m")"(help) "RL_ESC_SEQ("\x1b[95m")"> "RL_ESC_SEQ("\x1b[0m")
#endif

void vm_debug(struct vm *p_vm);

#endif
