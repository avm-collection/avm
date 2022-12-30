#ifndef LOADER_H__HEADER_GUARD__
#define LOADER_H__HEADER_GUARD__

#include <string.h>  /* strncmp, strerror */
#include <errno.h>   /* errno */
#include <assert.h>  /* assert */
#include <stdlib.h>  /* free, malloc, exit, EXIT_FAILURE */
#include <stdio.h>   /* stderr, FILE, fopen, fclose, fread, fgetc, ungetc */

#include "avm/vm.h"
#include "debugger.h"

void vm_exec_from_file(struct vm *p_vm, const char *p_path, bool p_warnings, bool p_debug);

#endif
