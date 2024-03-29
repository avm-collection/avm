#include "vm.h"

const char *err_to_str[] = {
	[ERR_OK]                   = "OK",
	[ERR_STACK_OVERFLOW]       = "Stack overflow",
	[ERR_STACK_UNDERFLOW]      = "Stack underflow",
	[ERR_CALL_STACK_OVERFLOW]  = "Call stack overflow",
	[ERR_CALL_STACK_UNDERFLOW] = "Call stack underflow",
	[ERR_INVALID_INST]         = "Invalid instruction",
	[ERR_INVALID_INST_ACCESS]  = "Invalid instruction access",
	[ERR_INVALID_MEM_ACCESS]   = "Invalid memory access",
	[ERR_DIV_BY_ZERO]          = "Division by zero",
	[ERR_MAX_FILES_OPEN]       = "Reached max limit of files open",
	[ERR_INVALID_FMODE]        = "Invalid file mode",
	[ERR_INVALID_DESCRIPTOR]   = "Invalid descriptor",
	[ERR_MAX_LIBS_OPEN]        = "Reached max limit of libraries open",
	[ERR_MAX_FUNCS_LOADED]     = "Reached max limit of functions loaded",
};

#define FMODE_STR_SIZE 4

char *fmode_to_str(enum fmode p_fmode) {
	char *str = (char*)malloc(FMODE_STR_SIZE);
	if (str == NULL) {
		VM_ERROR(stderr, "malloc() fail near "__FILE__":%i", __LINE__);
		exit(EXIT_FAILURE);
	}

	memset(str, 0, FMODE_STR_SIZE);

	if ((p_fmode & FMODE_READ) && (p_fmode & FMODE_WRITE)) {
		str[0] = p_fmode & FMODE_APPEND? 'r' : 'w';

		int next = 1;
		if (p_fmode & FMODE_BINARY) {
			str[next] = 'b';
			++ next;
		}

		str[next] = '+';
	} else {
		if (p_fmode & FMODE_READ)
			str[0] = 'r';
		else if (p_fmode & FMODE_APPEND)
			str[0] = 'a';
		else if (p_fmode & FMODE_WRITE)
			str[0] = 'w';
		else {
			free(str);

			return NULL;
		}

		if (p_fmode & FMODE_BINARY)
			str[1] = 'b';
	}

	return str;
}

void vm_init(struct vm *p_vm) {
	memset(p_vm, 0, sizeof(struct vm));

	p_vm->stack = (value_t*)malloc(STACK_SIZE_BYTES);
	if (p_vm->stack == NULL) {
		VM_ERROR(stderr, "malloc() fail near "__FILE__":%i", __LINE__);
		exit(EXIT_FAILURE);
	}

	p_vm->call_stack = (word_t*)malloc(CALL_STACK_SIZE_BYTES);
	if (p_vm->call_stack == NULL) {
		VM_ERROR(stderr, "malloc() fail near "__FILE__":%i", __LINE__);
		exit(EXIT_FAILURE);
	}

	p_vm->maps = (struct maps*)malloc(sizeof(*p_vm->maps));
	if (p_vm->maps == NULL) {
		VM_ERROR(stderr, "malloc() fail near "__FILE__":%i", __LINE__);
		exit(EXIT_FAILURE);
	}
	memset(p_vm->maps, 0, sizeof(*p_vm->maps));

	p_vm->maps->files[0].file = stdin;
	p_vm->maps->files[0].mode = FMODE_READ;

	p_vm->maps->files[1].file = stdout;
	p_vm->maps->files[1].mode = FMODE_WRITE;

	p_vm->maps->files[2].file = stderr;
	p_vm->maps->files[2].mode = FMODE_WRITE;
}

void vm_alloc_mem(struct vm *p_vm, word_t p_bytes) {
	p_vm->memory_size = p_bytes;

	if (p_vm->memory == NULL) {
		p_vm->memory = (uint8_t*)malloc(p_bytes);
		if (p_vm->memory == NULL) {
			VM_ERROR(stderr, "malloc() fail near "__FILE__":%i", __LINE__);
			exit(EXIT_FAILURE);
		}
	} else {
		p_vm->memory = (uint8_t*)realloc(p_vm->memory, p_bytes);
		if (p_vm->memory == NULL) {
			VM_ERROR(stderr, "realloc() fail near "__FILE__":%i", __LINE__);
			exit(EXIT_FAILURE);
		}
	}
}

void vm_destroy(struct vm *p_vm) {
	free(p_vm->stack);
	free(p_vm->call_stack);
	free(p_vm->maps);

	if (p_vm->memory != NULL)
		free(p_vm->memory);
}

#define STACK_ARGS_COUNT(P_COUNT) \
	if (p_vm->sp < P_COUNT) \
		return ERR_STACK_UNDERFLOW

int vm_exec_next_inst(struct vm *p_vm) {
	struct inst *inst = &p_vm->program[p_vm->ip];

	switch (inst->op) {
	case OP_NOP: break;

	case OP_PSH:
		if (p_vm->sp >= STACK_CAPACITY)
			return ERR_STACK_OVERFLOW;

		p_vm->stack[p_vm->sp ++].u64 = inst->data.u64;

		break;

	case OP_POP: STACK_ARGS_COUNT(1);
		-- p_vm->sp;

		break;

	case OP_ADD: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 += vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_SUB: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 -= vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_MUL: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 *= vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_DIV: STACK_ARGS_COUNT(2); {
		word_t b = vm_stack_top(p_vm, 0)->u64;
		if (b == 0)
			return ERR_DIV_BY_ZERO;

		vm_stack_top(p_vm, 1)->u64 /= b;
		-- p_vm->sp;
	} break;

	case OP_MOD: STACK_ARGS_COUNT(2);
		word_t b = vm_stack_top(p_vm, 0)->u64;
		if (b == 0)
			return ERR_DIV_BY_ZERO;

		vm_stack_top(p_vm, 1)->u64 %= b;
		-- p_vm->sp;

		break;

	case OP_INC: STACK_ARGS_COUNT(1);
		++ vm_stack_top(p_vm, 0)->u64;

		break;

	case OP_DEC: STACK_ARGS_COUNT(1);
		-- vm_stack_top(p_vm, 0)->u64;

		break;

	case OP_FAD: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->f64 += vm_stack_top(p_vm, 0)->f64;
		-- p_vm->sp;

		break;

	case OP_FSB: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->f64 -= vm_stack_top(p_vm, 0)->f64;
		-- p_vm->sp;

		break;

	case OP_FMU: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->f64 *= vm_stack_top(p_vm, 0)->f64;
		-- p_vm->sp;

		break;

	case OP_FDI: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->f64 /= vm_stack_top(p_vm, 0)->f64;
		-- p_vm->sp;

		break;

	case OP_FIN: STACK_ARGS_COUNT(1);
		++ vm_stack_top(p_vm, 0)->f64;

		break;

	case OP_FDE: STACK_ARGS_COUNT(1);
		-- vm_stack_top(p_vm, 0)->f64;

		break;

	case OP_NEG: STACK_ARGS_COUNT(1);
		vm_stack_top(p_vm, 0)->i64 = -vm_stack_top(p_vm, 0)->i64;

		break;

	case OP_NOT: STACK_ARGS_COUNT(1);
		vm_stack_top(p_vm, 0)->u64 = !vm_stack_top(p_vm, 0)->u64;

		break;

	case OP_JMP:
		if (inst->data.u64 >= p_vm->program_size)
			return ERR_INVALID_INST_ACCESS;

		p_vm->ip = inst->data.u64 - 1;

		break;

	case OP_JNZ: STACK_ARGS_COUNT(1);
		if (vm_stack_top(p_vm, 0)->u64) {
			if (inst->data.u64 >= p_vm->program_size)
				return ERR_INVALID_INST_ACCESS;

			p_vm->ip = inst->data.u64 - 1;
		}

		-- p_vm->sp;

		break;

	case OP_EQU: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->i64 == vm_stack_top(p_vm, 0)->i64;
		-- p_vm->sp;

		break;

	case OP_NEQ: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->i64 != vm_stack_top(p_vm, 0)->i64;
		-- p_vm->sp;

		break;

	case OP_GRT: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->i64 > vm_stack_top(p_vm, 0)->i64;
		-- p_vm->sp;

		break;

	case OP_GEQ: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->i64 >= vm_stack_top(p_vm, 0)->i64;
		-- p_vm->sp;

		break;

	case OP_LES: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->i64 < vm_stack_top(p_vm, 0)->i64;
		-- p_vm->sp;

		break;

	case OP_LEQ: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->i64 <= vm_stack_top(p_vm, 0)->i64;
		-- p_vm->sp;

		break;

	case OP_UEQ: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->u64 == vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_UNE: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->u64 != vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_UGR: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->u64 > vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_UGQ: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->u64 >= vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_ULE: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->u64 == vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_ULQ: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->u64 <= vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_FEQ: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->f64 == vm_stack_top(p_vm, 0)->f64;
		-- p_vm->sp;

		break;

	case OP_FNE: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->f64 != vm_stack_top(p_vm, 0)->f64;
		-- p_vm->sp;

		break;

	case OP_FGR: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->f64 > vm_stack_top(p_vm, 0)->f64;
		-- p_vm->sp;

		break;

	case OP_FGQ: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->f64 >= vm_stack_top(p_vm, 0)->f64;
		-- p_vm->sp;

		break;

	case OP_FLE: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->f64 < vm_stack_top(p_vm, 0)->f64;
		-- p_vm->sp;

		break;

	case OP_FLQ: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->f64 <= vm_stack_top(p_vm, 0)->f64;
		-- p_vm->sp;

		break;

	case OP_CAL:
		if (inst->data.u64 >= p_vm->program_size)
			return ERR_INVALID_INST_ACCESS;
		else if (p_vm->cs >= CALL_STACK_CAPACITY)
			return ERR_CALL_STACK_OVERFLOW;

		p_vm->call_stack[p_vm->cs ++] = p_vm->ip + 1;
		p_vm->ip = inst->data.u64 - 1;

		break;

	case OP_RET:
		if (p_vm->cs <= 0)
			return ERR_CALL_STACK_UNDERFLOW;

		p_vm->ip = p_vm->call_stack[-- p_vm->cs] - 1;

		break;

	case OP_AND: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->u64 && vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_ORR: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 = vm_stack_top(p_vm, 1)->u64 || vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_DUP: STACK_ARGS_COUNT(inst->data.u64 + 1);
		if (p_vm->sp >= STACK_CAPACITY)
			return ERR_STACK_OVERFLOW;

		++ p_vm->sp;
		vm_stack_top(p_vm, 0)->u64 = vm_stack_top(p_vm, inst->data.u64 + 1)->u64;

		break;

	case OP_SWP: STACK_ARGS_COUNT(inst->data.u64 + 2); {
		word_t tmp = vm_stack_top(p_vm, 0)->u64;
		vm_stack_top(p_vm, 0)->u64 = vm_stack_top(p_vm, inst->data.u64 + 1)->u64;
		vm_stack_top(p_vm, inst->data.u64 + 1)->u64 = tmp;
	} break;

	case OP_EMP:
		if (p_vm->sp >= STACK_CAPACITY)
			return ERR_STACK_OVERFLOW;

		++ p_vm->sp;
		vm_stack_top(p_vm, 0)->u64 = p_vm->sp <= 0;

		break;

	case OP_SET: STACK_ARGS_COUNT(3); {
		word_t  addr = vm_stack_top(p_vm, 2)->u64;
		uint8_t val  = vm_stack_top(p_vm, 1)->u64;
		word_t  size = vm_stack_top(p_vm, 0)->u64;

		if (!vm_is_chunk_valid(p_vm, addr, size))
			return ERR_INVALID_MEM_ACCESS;

		p_vm->sp -= 3;

		memset(&p_vm->memory[addr], val, size);
	} break;

	case OP_CPY: STACK_ARGS_COUNT(3); {
		word_t to   = vm_stack_top(p_vm, 2)->u64;
		word_t from = vm_stack_top(p_vm, 1)->u64;
		word_t size = vm_stack_top(p_vm, 0)->u64;

		if (!vm_is_chunk_valid(p_vm, to, size) || !vm_is_chunk_valid(p_vm, from, size))
			return ERR_INVALID_MEM_ACCESS;

		p_vm->sp -= 3;

		memcpy(&p_vm->memory[to], &p_vm->memory[from], size);
	} break;

	case OP_R08: STACK_ARGS_COUNT(1); {
		word_t addr = vm_stack_top(p_vm, 0)->u64;

		uint8_t data;
		enum err ret = vm_read8(p_vm, &data, addr);
		if (ret != ERR_OK)
			return ret;

		vm_stack_top(p_vm, 0)->u64 = data;
	} break;

	case OP_R16: STACK_ARGS_COUNT(1); {
		word_t addr = vm_stack_top(p_vm, 0)->u64;

		uint16_t data;
		enum err ret = vm_read16(p_vm, &data, addr);
		if (ret != ERR_OK)
			return ret;

		vm_stack_top(p_vm, 0)->u64 = data;
	} break;

	case OP_R32: STACK_ARGS_COUNT(1); {
		word_t addr = vm_stack_top(p_vm, 0)->u64;

		uint32_t data;
		enum err ret = vm_read32(p_vm, &data, addr);
		if (ret != ERR_OK)
			return ret;

		vm_stack_top(p_vm, 0)->u64 = data;
	} break;

	case OP_R64: STACK_ARGS_COUNT(1); {
		word_t addr = vm_stack_top(p_vm, 0)->u64;

		uint64_t data;
		enum err ret = vm_read64(p_vm, &data, addr);
		if (ret != ERR_OK)
			return ret;

		vm_stack_top(p_vm, 0)->u64 = data;
	} break;

	case OP_W08: STACK_ARGS_COUNT(2); {
		word_t  addr = vm_stack_top(p_vm, 1)->u64;
		uint8_t data = vm_stack_top(p_vm, 0)->u64;

		enum err ret = vm_write8(p_vm, data, addr);
		if (ret != ERR_OK)
			return ret;

		p_vm->sp -= 2;
	} break;

	case OP_W16: STACK_ARGS_COUNT(2); {
		word_t   addr = vm_stack_top(p_vm, 1)->u64;
		uint16_t data = vm_stack_top(p_vm, 0)->u64;

		enum err ret = vm_write16(p_vm, data, addr);
		if (ret != ERR_OK)
			return ret;

		p_vm->sp -= 2;
	} break;

	case OP_W32: STACK_ARGS_COUNT(2); {
		word_t   addr = vm_stack_top(p_vm, 1)->u64;
		uint32_t data = vm_stack_top(p_vm, 0)->u64;

		enum err ret = vm_write32(p_vm, data, addr);
		if (ret != ERR_OK)
			return ret;

		p_vm->sp -= 2;
	} break;

	case OP_W64: STACK_ARGS_COUNT(2); {
		word_t   addr = vm_stack_top(p_vm, 1)->u64;
		uint64_t data = vm_stack_top(p_vm, 0)->u64;

		enum err ret = vm_write64(p_vm, data, addr);
		if (ret != ERR_OK)
			return ret;

		p_vm->sp -= 2;
	} break;

	case OP_OPE: STACK_ARGS_COUNT(3); {
		word_t     addr = vm_stack_top(p_vm, 2)->u64;
		word_t     size = vm_stack_top(p_vm, 1)->u64;
		enum fmode mode = vm_stack_top(p_vm, 0)->u64;

		char name[size + 1];
		vm_get_str(p_vm, name, addr, size);

		p_vm->sp -= 2;

		word_t fd = vm_get_free_fd(p_vm);
		if (fd == INVALID_DESCRIPTOR)
			return ERR_MAX_FILES_OPEN;

		struct file *f = &p_vm->maps->files[fd];

		char *mode_str = fmode_to_str(mode);
		if (mode_str == NULL)
			return ERR_INVALID_FMODE;

		f->mode = mode;
		f->file = fopen(name, mode_str);
		vm_stack_top(p_vm, 0)->u64 = f->file == NULL? INVALID_DESCRIPTOR : fd;

		free(mode_str);
	} break;

	case OP_CLO: STACK_ARGS_COUNT(1); {
		word_t fd = p_vm->stack[-- p_vm->sp].u64;
		if (!vm_is_fd_valid(p_vm, fd))
			return ERR_INVALID_DESCRIPTOR;

		struct file *f = &p_vm->maps->files[fd];

		fclose(f->file);
		f->file = NULL;
	} break;

	case OP_WRF: STACK_ARGS_COUNT(2); {
		word_t addr = vm_stack_top(p_vm, 2)->u64;
		word_t size = vm_stack_top(p_vm, 1)->u64;
		word_t fd   = vm_stack_top(p_vm, 0)->u64;

		if (!vm_is_chunk_valid(p_vm, addr, size))
			return ERR_INVALID_MEM_ACCESS;
		else if (!vm_is_fd_valid(p_vm, fd))
			return ERR_INVALID_DESCRIPTOR;

		p_vm->sp -= 2;

		size_t ret = fwrite(&p_vm->memory[addr], 1, size, p_vm->maps->files[fd].file);
		vm_stack_top(p_vm, 0)->u64 = (word_t)(ret < 1);
	} break;

	case OP_RDF: STACK_ARGS_COUNT(3); {
		word_t addr = vm_stack_top(p_vm, 2)->u64;
		word_t size = vm_stack_top(p_vm, 1)->u64;
		word_t fd   = vm_stack_top(p_vm, 0)->u64;

		if (!vm_is_chunk_valid(p_vm, addr, size))
			return ERR_INVALID_MEM_ACCESS;
		else if (!vm_is_fd_valid(p_vm, fd))
			return ERR_INVALID_DESCRIPTOR;

		p_vm->sp -= 2;

		size_t ret = fread(&p_vm->memory[addr], 1, size, p_vm->maps->files[fd].file);
		vm_stack_top(p_vm, 0)->u64 = (word_t)(ret < 1);
	} break;

	case OP_SZF: STACK_ARGS_COUNT(1); {
		word_t fd = vm_stack_top(p_vm, 0)->u64;
		if (!vm_is_fd_valid(p_vm, fd))
			return ERR_INVALID_DESCRIPTOR;

		struct file *f = &p_vm->maps->files[fd];

		fseek(f->file, 0, SEEK_END);
		vm_stack_top(p_vm, 0)->u64 = ftell(f->file);
		fseek(f->file, 0, SEEK_SET);
	} break;

	case OP_FLU: STACK_ARGS_COUNT(1); {
		word_t fd = p_vm->stack[-- p_vm->sp].u64;
		if (!vm_is_fd_valid(p_vm, fd))
			return ERR_INVALID_DESCRIPTOR;

		fflush(p_vm->maps->files[fd].file);
	} break;

	case OP_BAN: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 &= vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_BOR: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 |= vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_BSR: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 >>= vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_BSL: STACK_ARGS_COUNT(2);
		vm_stack_top(p_vm, 1)->u64 <<= vm_stack_top(p_vm, 0)->u64;
		-- p_vm->sp;

		break;

	case OP_LOL: STACK_ARGS_COUNT(2); {
		word_t addr = vm_stack_top(p_vm, 1)->u64;
		word_t size = vm_stack_top(p_vm, 0)->u64;

		char name[size + 1];
		vm_get_str(p_vm, name, addr, size);

		-- p_vm->sp;

		word_t ld = vm_get_free_ld(p_vm);
		if (ld == INVALID_DESCRIPTOR)
			return ERR_MAX_FILES_OPEN;

		struct lib *lib = &p_vm->maps->libs[ld];

		lib->handle = dlopen(name, RTLD_NOW | RTLD_GLOBAL);
		vm_stack_top(p_vm, 0)->u64 = lib->handle == NULL? INVALID_DESCRIPTOR : ld;
	} break;

	case OP_CLL: STACK_ARGS_COUNT(1); {
		word_t ld = p_vm->stack[-- p_vm->sp].u64;
		if (!vm_is_ld_valid(p_vm, ld))
			return ERR_INVALID_DESCRIPTOR;

		struct lib *lib = &p_vm->maps->libs[ld];

		dlclose(lib->handle);
		lib->handle = NULL;
		memset(lib->funcs, 0, sizeof(lib->funcs));
	} break;

	case OP_LLF: STACK_ARGS_COUNT(3); {
		word_t addr = vm_stack_top(p_vm, 2)->u64;
		word_t size = vm_stack_top(p_vm, 1)->u64;
		word_t ld   = vm_stack_top(p_vm, 0)->u64;

		if (!vm_is_chunk_valid(p_vm, addr, size))
			return ERR_INVALID_MEM_ACCESS;

		char name[size + 1];
		strncpy(name, (char*)&p_vm->memory[addr], size);
		name[size] = 0;

		p_vm->sp -= 2;

		word_t fnd = vm_get_free_fnd(p_vm, ld);
		if (fnd == INVALID_DESCRIPTOR)
			return ERR_MAX_FUNCS_LOADED;

		external_t *func = &p_vm->maps->libs[ld].funcs[fnd];

		*(void**)func = dlsym(p_vm->maps->libs[ld].handle, name);
		vm_stack_top(p_vm, 0)->u64 = *func == NULL? INVALID_DESCRIPTOR : fnd;
	} break;

	case OP_ULF: STACK_ARGS_COUNT(2); {
		word_t fnd = vm_stack_top(p_vm, 1)->u64;
		word_t ld  = vm_stack_top(p_vm, 0)->u64;
		if (!vm_is_ld_valid(p_vm, ld) || !vm_is_fnd_valid(p_vm, ld, fnd))
			return ERR_INVALID_DESCRIPTOR;

		p_vm->maps->libs[ld].funcs[fnd] = NULL;

		p_vm->sp -= 2;
	} break;

	case OP_CLF: STACK_ARGS_COUNT(2); {
		word_t fnd = vm_stack_top(p_vm, 1)->u64;
		word_t ld  = vm_stack_top(p_vm, 0)->u64;

		if (!vm_is_ld_valid(p_vm, ld) || !vm_is_fnd_valid(p_vm, ld, fnd))
			return ERR_INVALID_DESCRIPTOR;

		p_vm->sp -= 2;

		enum err ret = p_vm->maps->libs[ld].funcs[fnd](p_vm);
		if (ret != ERR_OK)
			return ret;
	} break;

	case OP_DMP:
		putchar('\n');
		vm_dump(p_vm, stdout);
		fflush(stdout);

		break;

	case OP_PRT: STACK_ARGS_COUNT(1);
		printf("%i\n", (int)p_vm->stack[-- p_vm->sp].i64);

		break;

	case OP_FPR: STACK_ARGS_COUNT(1);
		printf("%f\n", (float)p_vm->stack[-- p_vm->sp].f64);

		break;

	case OP_HLT: STACK_ARGS_COUNT(1);
		p_vm->ex   = p_vm->stack[-- p_vm->sp].u64;
		p_vm->halt = true;

		break;

	default: return ERR_INVALID_INST;
	};

	++ p_vm->ip;

	return ERR_OK;
}

void vm_load_from_mem(struct vm *p_vm, struct inst *p_program, word_t p_size, word_t p_ep) {
	p_vm->program      = p_program;
	p_vm->program_size = p_size;
	p_vm->ip           = p_ep;
}

void vm_run(struct vm *p_vm) {
	while (p_vm->ip < p_vm->program_size && !p_vm->halt) {
		int ret = vm_exec_next_inst(p_vm);
		if (ret != ERR_OK)
			vm_panic(p_vm, ret);
	}
}

void vm_dump(struct vm *p_vm, FILE *p_file) {
	vm_dump_regs(p_vm, p_file);
	vm_dump_stack(p_vm, p_file);
	vm_dump_call_stack(p_vm, p_file);
	vm_dump_at(p_vm, p_file);
}

static void dump_reg(const char *p_name, word_t p_value, FILE *p_file) {
	set_fg_color(COLOR_GREY, p_file);
	fprintf(p_file, "%s |", p_name);
	set_fg_color(COLOR_DEFAULT, p_file);
	fprintf(p_file, " 0x%"FMT_HEX"\n", AS_FMT_HEX(p_value));
}

void vm_dump_regs(struct vm *p_vm, FILE *p_file) {
	VM_NOTE(stderr, "Registers:");

	set_fg_color(COLOR_DEFAULT, p_file);
	dump_reg("IP", p_vm->ip, p_file);
	dump_reg("SP", p_vm->sp, p_file);
	dump_reg("CS", p_vm->cs, p_file);
	dump_reg("EX", p_vm->ex, p_file);

	set_fg_color(COLOR_DEFAULT, p_file);
}

void vm_dump_stack_top(struct vm *p_vm, FILE *p_file) {
	VM_NOTE(stderr, "Stack top:");

	if (p_vm->sp <= 0) {
		set_fg_color(COLOR_GREY, p_file);
		fputs("EMPTY\n", p_file);
		set_fg_color(COLOR_DEFAULT, p_file);
	} else {
		set_fg_color(COLOR_DEFAULT, p_file);
		fprintf(p_file, "0x%"FMT_HEX" (%f | %lli)\n", AS_FMT_HEX(p_vm->stack[p_vm->sp - 1].u64),
		        p_vm->stack[p_vm->sp - 1].f64, (long long)p_vm->stack[p_vm->sp - 1].i64);
	}
}

void vm_dump_stack(struct vm *p_vm, FILE *p_file) {
	VM_NOTE(stderr, "Stack:");

	if (p_vm->sp <= 0) {
		set_fg_color(COLOR_GREY, p_file);
		fputs("EMPTY\n", p_file);
		set_fg_color(COLOR_DEFAULT, p_file);

		return;
	}

	for (word_t i = 0; i < p_vm->sp; ++ i) {
		if (i % 3 == 0) {
			if (i > 0)
				fputc('\n', p_file);

			set_fg_color(COLOR_GREY, p_file);
			fprintf(p_file,"0x%"FMT_HEX" |", AS_FMT_HEX(i * sizeof(word_t)));
			set_fg_color(COLOR_DEFAULT, p_file);
		}

		fprintf(p_file, " 0x%"FMT_HEX, AS_FMT_HEX(p_vm->stack[i].u64));
	}
	fputc('\n', p_file);
}

void vm_dump_call_stack(struct vm *p_vm, FILE *p_file) {
	VM_NOTE(stderr, "Call stack:");

	if (p_vm->cs <= 0) {
		set_fg_color(COLOR_GREY, p_file);
		fputs("EMPTY\n", p_file);
		set_fg_color(COLOR_DEFAULT, p_file);

		return;
	}

	for (word_t i = 0; i < p_vm->cs; ++ i) {
		set_fg_color(COLOR_GREY, p_file);
		fputs("from ", p_file);

		set_fg_color(COLOR_DEFAULT, p_file);
		fprintf(p_file, "0x%"FMT_HEX"\n", AS_FMT_HEX(p_vm->call_stack[i]));
	}
}

void vm_dump_at(struct vm *p_vm, FILE *p_file) {
	set_fg_color(COLOR_GREY, p_file);
	fputs("  -> ", p_file);
	set_fg_color(COLOR_DEFAULT, p_file);
	fputs("at ", p_file);
	set_fg_color(COLOR_BRIGHT_WHITE, p_file);
	set_bg_color(COLOR_MAGENTA, p_file);
	fprintf(p_file, "0x%"FMT_HEX, AS_FMT_HEX(p_vm->ip));
	set_fg_color(COLOR_DEFAULT, p_file);
	set_bg_color(COLOR_DEFAULT, p_file);
	fputc('\n', p_file);
}

void vm_panic(struct vm *p_vm, enum err p_err) {
	fputc('\n', stderr);
	VM_ERROR(stderr, err_to_str[p_err]);
	vm_dump_at(p_vm, stderr);

	if (p_vm->cs > 0)
		vm_dump_call_stack(p_vm, stderr);

	exit(p_err);
}

void log_colored(FILE *p_file, enum color p_color, const char *p_fmt, ...) {
	PARSE_FMT_INTO(p_fmt, msg, 256);

	set_fg_color(p_color, p_file);
	fprintf(p_file, "["TITLE"] %s\n", msg);
	set_fg_color(COLOR_DEFAULT, p_file);
}

enum err vm_read8(struct vm *p_vm, uint8_t *p_data, word_t p_addr) {
	if (!vm_is_chunk_valid(p_vm, p_addr, sizeof(uint8_t)))
		return ERR_INVALID_MEM_ACCESS;

	*p_data = p_vm->memory[p_addr];

	return ERR_OK;
}

enum err vm_read16(struct vm *p_vm, uint16_t *p_data, word_t p_addr) {
	if (!vm_is_chunk_valid(p_vm, p_addr, sizeof(uint16_t)))
		return ERR_INVALID_MEM_ACCESS;

	*p_data = ((uint16_t)p_vm->memory[p_addr] << 010) |
	           (uint16_t)p_vm->memory[p_addr + 1];

	return ERR_OK;
}

enum err vm_read32(struct vm *p_vm, uint32_t *p_data, word_t p_addr) {
	if (!vm_is_chunk_valid(p_vm, p_addr, sizeof(uint32_t)))
		return ERR_INVALID_MEM_ACCESS;

	*p_data = ((uint32_t)p_vm->memory[p_addr]     << 030) |
	          ((uint32_t)p_vm->memory[p_addr + 1] << 020) |
	          ((uint32_t)p_vm->memory[p_addr + 2] << 010) |
	           (uint32_t)p_vm->memory[p_addr + 3];

	return ERR_OK;
}

enum err vm_read64(struct vm *p_vm, uint64_t *p_data, word_t p_addr) {
	if (!vm_is_chunk_valid(p_vm, p_addr, sizeof(uint64_t)))
		return ERR_INVALID_MEM_ACCESS;

	*p_data = ((uint64_t)p_vm->memory[p_addr]     << 070) |
	          ((uint64_t)p_vm->memory[p_addr + 1] << 060) |
	          ((uint64_t)p_vm->memory[p_addr + 2] << 050) |
	          ((uint64_t)p_vm->memory[p_addr + 3] << 040) |
	          ((uint64_t)p_vm->memory[p_addr + 4] << 030) |
	          ((uint64_t)p_vm->memory[p_addr + 5] << 020) |
	          ((uint64_t)p_vm->memory[p_addr + 6] << 010) |
	           (uint64_t)p_vm->memory[p_addr + 7];

	return ERR_OK;
}

enum err vm_write8(struct vm *p_vm, uint8_t p_data, word_t p_addr) {
	if (!vm_is_chunk_valid(p_vm, p_addr, sizeof(uint8_t)))
		return ERR_INVALID_MEM_ACCESS;

	p_vm->memory[p_addr] = p_data;

	return ERR_OK;
}

enum err vm_write16(struct vm *p_vm, uint16_t p_data, word_t p_addr) {
	if (!vm_is_chunk_valid(p_vm, p_addr, sizeof(uint16_t)))
		return ERR_INVALID_MEM_ACCESS;

	p_vm->memory[p_addr]     = (p_data & 0xFF00) >> 010;
	p_vm->memory[p_addr + 1] =  p_data & 0x00FF;

	return ERR_OK;
}

enum err vm_write32(struct vm *p_vm, uint32_t p_data, word_t p_addr) {
	if (!vm_is_chunk_valid(p_vm, p_addr, sizeof(uint32_t)))
		return ERR_INVALID_MEM_ACCESS;

	p_vm->memory[p_addr]     = (p_data & 0xFF000000) >> 030;
	p_vm->memory[p_addr + 1] = (p_data & 0x00FF0000) >> 020;
	p_vm->memory[p_addr + 2] = (p_data & 0x0000FF00) >> 010;
	p_vm->memory[p_addr + 3] =  p_data & 0x000000FF;

	return ERR_OK;
}

enum err vm_write64(struct vm *p_vm, uint64_t p_data, word_t p_addr) {
	if (!vm_is_chunk_valid(p_vm, p_addr, sizeof(uint64_t)))
		return ERR_INVALID_MEM_ACCESS;

	p_vm->memory[p_addr]     = (p_data & 0xFF00000000000000) >> 070;
	p_vm->memory[p_addr + 1] = (p_data & 0x00FF000000000000) >> 060;
	p_vm->memory[p_addr + 2] = (p_data & 0x0000FF0000000000) >> 050;
	p_vm->memory[p_addr + 3] = (p_data & 0x000000FF00000000) >> 040;
	p_vm->memory[p_addr + 4] = (p_data & 0x00000000FF000000) >> 030;
	p_vm->memory[p_addr + 5] = (p_data & 0x0000000000FF0000) >> 020;
	p_vm->memory[p_addr + 6] = (p_data & 0x000000000000FF00) >> 010;
	p_vm->memory[p_addr + 7] =  p_data & 0x00000000000000FF;

	return ERR_OK;
}

word_t vm_get_free_fd(struct vm *p_vm) {
	for (word_t i = 0; i < MAX_OPEN_FILES; ++ i) {
		if (p_vm->maps->files[i].file == NULL)
			return i;
	}

	return INVALID_DESCRIPTOR;
}

word_t vm_get_free_ld(struct vm *p_vm) {
	for (word_t i = 0; i < MAX_OPEN_LIBS; ++ i) {
		if (p_vm->maps->libs[i].handle == NULL)
			return i;
	}

	return INVALID_DESCRIPTOR;
}

word_t vm_get_free_fnd(struct vm *p_vm, word_t p_ld) {
	for (word_t i = 0; i < MAX_LOADED_FUNCS; ++ i) {
		if (p_vm->maps->libs[p_ld].funcs[i] == NULL)
			return i;
	}

	return INVALID_DESCRIPTOR;
}

bool vm_get_str(struct vm *p_vm, char *p_buf, word_t p_addr, word_t p_size) {
	if (!vm_is_chunk_valid(p_vm, p_addr, p_size))
		return false;

	strncpy(p_buf, (char*)&p_vm->memory[p_addr], p_size);
	p_buf[p_size] = 0;

	return true;
}

bool vm_is_fd_valid(struct vm *p_vm, word_t p_fd) {
	return p_fd < MAX_OPEN_FILES && p_vm->maps->files[p_fd].file != NULL;
}

bool vm_is_ld_valid(struct vm *p_vm, word_t p_ld) {
	return p_ld < MAX_OPEN_LIBS && p_vm->maps->libs[p_ld].handle != NULL;
}

bool vm_is_fnd_valid(struct vm *p_vm, word_t p_ld, word_t p_fnd) {
	return p_fnd < MAX_LOADED_FUNCS && p_vm->maps->libs[p_ld].funcs[p_fnd] != NULL;
}

bool vm_is_chunk_valid(struct vm *p_vm, word_t p_addr, word_t p_size) {
	return p_addr < p_vm->memory_size - p_size + 1;
}

value_t *vm_stack_top(struct vm *p_vm, word_t p_off) {
	return &p_vm->stack[p_vm->sp - p_off - 1];
}
