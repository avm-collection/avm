#ifndef LOADER_H__HEADER_GUARD__
#define LOADER_H__HEADER_GUARD__

#include "avm/vm.h"
#include "debugger.h"

void vm_exec_from_file(struct vm *p_vm, const char *p_path, bool p_warnings, bool p_debug);

#endif
