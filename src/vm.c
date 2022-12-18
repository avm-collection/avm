#include "vm.h"

const char *op_to_str[] = {
	[OP_NOP] = "NOP",

	[OP_PSH] = "PSH",
	[OP_POP] = "POP",

	[OP_ADD] = "ADD",
	[OP_SUB] = "SUB",

	[OP_MUL] = "MUL",
	[OP_DIV] = "DIV",
	[OP_MOD] = "MOD",

	[OP_INC] = "INC",
	[OP_DEC] = "DEC",

	[OP_FAD] = "FAD",
	[OP_FSB] = "FSB",

	[OP_FMU] = "FMU",
	[OP_FDI] = "FDI",

	[OP_FIN] = "FIN",
	[OP_FDE] = "FDE",

	[OP_NEG] = "NEG",
	[OP_NOT] = "NOT",

	[OP_JMP] = "JMP",
	[OP_JNZ] = "JNZ",

	[OP_CAL] = "CAL",
	[OP_RET] = "RET",

	[OP_EQU] = "EQU",
	[OP_NEQ] = "NEQ",
	[OP_GRT] = "GRT",
	[OP_GEQ] = "GEQ",
	[OP_LES] = "LES",
	[OP_LEQ] = "LEQ",

	[OP_UEQ] = "UEQ",
	[OP_UNE] = "UNE",
	[OP_UGR] = "UGR",
	[OP_UGQ] = "UGQ",
	[OP_ULE] = "ULE",
	[OP_ULQ] = "ULQ",

	[OP_FEQ] = "FEQ",
	[OP_FNE] = "FNE",
	[OP_FGR] = "FGR",
	[OP_FGQ] = "FGQ",
	[OP_FLE] = "FLE",
	[OP_FLQ] = "FLQ",

	[OP_DUP] = "DUP",
	[OP_SWP] = "SWP",
	[OP_EMP] = "EMP",
	[OP_SET] = "SET",
	[OP_CPY] = "CPY",

	[OP_R08] = "R08",
	[OP_R16] = "R16",
	[OP_R32] = "R32",
	[OP_R64] = "R64",

	[OP_W08] = "W08",
	[OP_W16] = "W16",
	[OP_W32] = "W32",
	[OP_W64] = "W64",

	[OP_OPE] = "OPE",
	[OP_CLO] = "CLO",
	[OP_WRF] = "WRF",
	[OP_RDF] = "RDF",

	[OP_DMP] = "DMP",
	[OP_PRT] = "PRT",
	[OP_FPR] = "FPR",

	[OP_HLT] = "HLT",
};

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
	[ERR_INVALID_FD]           = "Invalid file descriptor",
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

void vm_init(struct vm *p_vm, bool p_warnings, bool p_debug) {
	memset(p_vm, 0, sizeof(struct vm));
	p_vm->warnings = p_warnings;
	p_vm->debug    = p_debug;

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

	p_vm->memory = (uint8_t*)malloc(MEMORY_SIZE_BYTES);
	if (p_vm->memory == NULL) {
		VM_ERROR(stderr, "malloc() fail near "__FILE__":%i", __LINE__);
		exit(EXIT_FAILURE);
	}

	p_vm->files = (struct file*)malloc(MAX_OPEN_FILES * sizeof(struct file));
	if (p_vm->files == NULL) {
		VM_ERROR(stderr, "malloc() fail near "__FILE__":%i", __LINE__);
		exit(EXIT_FAILURE);
	}
	memset(p_vm->files, 0, MAX_OPEN_FILES * sizeof(struct file));
}

void vm_destroy(struct vm *p_vm) {
	free(p_vm->stack);
	free(p_vm->call_stack);
	free(p_vm->memory);
	free(p_vm->files);
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
	if (p_addr >= MEMORY_SIZE_BYTES)
		return ERR_INVALID_MEM_ACCESS;

	*p_data = p_vm->memory[p_addr];

	return ERR_OK;
}

enum err vm_read16(struct vm *p_vm, uint16_t *p_data, word_t p_addr) {
	if (p_addr >= MEMORY_SIZE_BYTES - 1)
		return ERR_INVALID_MEM_ACCESS;

	*p_data = ((uint16_t)p_vm->memory[p_addr] << 010) |
	           (uint16_t)p_vm->memory[p_addr + 1];

	return ERR_OK;
}

enum err vm_read32(struct vm *p_vm, uint32_t *p_data, word_t p_addr) {
	if (p_addr >= MEMORY_SIZE_BYTES - 3)
		return ERR_INVALID_MEM_ACCESS;

	*p_data = ((uint32_t)p_vm->memory[p_addr]     << 030) |
	          ((uint32_t)p_vm->memory[p_addr + 1] << 020) |
	          ((uint32_t)p_vm->memory[p_addr + 2] << 010) |
	           (uint32_t)p_vm->memory[p_addr + 3];

	return ERR_OK;
}

enum err vm_read64(struct vm *p_vm, uint64_t *p_data, word_t p_addr) {
	if (p_addr >= MEMORY_SIZE_BYTES - 7)
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
	if (p_addr >= MEMORY_SIZE_BYTES)
		return ERR_INVALID_MEM_ACCESS;

	p_vm->memory[p_addr] = p_data;

	return ERR_OK;
}

enum err vm_write16(struct vm *p_vm, uint16_t p_data, word_t p_addr) {
	if (p_addr >= MEMORY_SIZE_BYTES - 1)
		return ERR_INVALID_MEM_ACCESS;

	p_vm->memory[p_addr]     = (p_data & 0xFF00) >> 010;
	p_vm->memory[p_addr + 1] =  p_data & 0x00FF;

	return ERR_OK;
}

enum err vm_write32(struct vm *p_vm, uint32_t p_data, word_t p_addr) {
	if (p_addr >= MEMORY_SIZE_BYTES - 3)
		return ERR_INVALID_MEM_ACCESS;

	p_vm->memory[p_addr]     = (p_data & 0xFF000000) >> 030;
	p_vm->memory[p_addr + 1] = (p_data & 0x00FF0000) >> 020;
	p_vm->memory[p_addr + 2] = (p_data & 0x0000FF00) >> 010;
	p_vm->memory[p_addr + 3] =  p_data & 0x000000FF;

	return ERR_OK;
}

enum err vm_write64(struct vm *p_vm, uint64_t p_data, word_t p_addr) {
	if (p_addr >= MEMORY_SIZE_BYTES - 7)
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

char *vm_mem_str_to_cstr(struct vm *p_vm, word_t p_addr) {
	word_t   size;
	enum err ret = vm_read64(p_vm, &size, p_addr);
	if (ret != ERR_OK)
		return NULL;

	if (p_addr >= MEMORY_SIZE_BYTES - size - 7)
		return NULL;

	char *str = malloc(size + 1);
	if (str == NULL) {
		VM_ERROR(stderr, "malloc() fail near "__FILE__":%i", __LINE__);
		exit(EXIT_FAILURE);
	}

	for (word_t i = 0; i < size; ++ i) {
		str[i] = p_vm->memory[p_addr + 8 + i];
	}

	str[size] = 0;

	return str;
}

static int vm_exec_next_inst(struct vm *p_vm) {
	struct inst *inst = &p_vm->program[p_vm->ip];

	switch (inst->op) {
	case OP_NOP: break;

	case OP_PSH:
		if (p_vm->sp >= STACK_CAPACITY)
			return ERR_STACK_OVERFLOW;

		p_vm->stack[p_vm->sp ++].u64 = inst->data.u64;

		break;

	case OP_POP:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		-- p_vm->sp;

		break;

	case OP_ADD:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 += p_vm->stack[p_vm->sp - 1].u64;
		-- p_vm->sp;

		break;

	case OP_SUB:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 -= p_vm->stack[p_vm->sp - 1].u64;
		-- p_vm->sp;

		break;

	case OP_MUL:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 *= p_vm->stack[p_vm->sp - 1].u64;
		-- p_vm->sp;

		break;

	case OP_DIV:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		{
			word_t b = p_vm->stack[p_vm->sp - 1].u64;
			if (b == 0)
				return ERR_DIV_BY_ZERO;

			p_vm->stack[p_vm->sp - 2].u64 /= b;
			-- p_vm->sp;
		}

		break;

	case OP_MOD:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 %= p_vm->stack[p_vm->sp - 1].u64;
		-- p_vm->sp;

		break;

	case OP_INC:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		++ p_vm->stack[p_vm->sp - 1].u64;

		break;

	case OP_DEC:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		-- p_vm->stack[p_vm->sp - 1].u64;

		break;

	case OP_FAD:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].f64 += p_vm->stack[p_vm->sp - 1].f64;
		-- p_vm->sp;

		break;

	case OP_FSB:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].f64 -= p_vm->stack[p_vm->sp - 1].f64;
		-- p_vm->sp;

		break;

	case OP_FMU:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].f64 *= p_vm->stack[p_vm->sp - 1].f64;
		-- p_vm->sp;

		break;

	case OP_FDI:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].f64 /= p_vm->stack[p_vm->sp - 1].f64;
		-- p_vm->sp;

		break;

	case OP_FIN:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		++ p_vm->stack[p_vm->sp - 1].f64;

		break;

	case OP_FDE:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		-- p_vm->stack[p_vm->sp - 1].f64;

		break;

	case OP_NEG:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 1].i64 = -p_vm->stack[p_vm->sp - 1].i64;

		break;

	case OP_NOT:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 1].u64 = !p_vm->stack[p_vm->sp - 1].u64;

		break;

	case OP_JMP:
		if (inst->data.u64 >= p_vm->program_size)
			return ERR_INVALID_INST_ACCESS;

		p_vm->ip = inst->data.u64 - 1;

		break;

	case OP_JNZ:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		if (p_vm->stack[p_vm->sp - 1].u64) {
			if (inst->data.u64 >= p_vm->program_size)
				return ERR_INVALID_INST_ACCESS;

			p_vm->ip = inst->data.u64 - 1;
		}

		-- p_vm->sp;

		break;

	case OP_EQU:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].i64 ==
		                                 p_vm->stack[p_vm->sp - 1].i64;
		-- p_vm->sp;

		break;

	case OP_NEQ:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].i64 !=
		                                 p_vm->stack[p_vm->sp - 1].i64;
		-- p_vm->sp;

		break;

	case OP_GRT:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].i64 >
		                                 p_vm->stack[p_vm->sp - 1].i64;
		-- p_vm->sp;

		break;

	case OP_GEQ:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].i64 >=
		                                 p_vm->stack[p_vm->sp - 1].i64;
		-- p_vm->sp;

		break;

	case OP_LES:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].i64 <
		                                 p_vm->stack[p_vm->sp - 1].i64;
		-- p_vm->sp;

		break;

	case OP_LEQ:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].i64 <=
		                                 p_vm->stack[p_vm->sp - 1].i64;
		-- p_vm->sp;

		break;

	case OP_UEQ:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].u64 ==
		                                 p_vm->stack[p_vm->sp - 1].u64;
		-- p_vm->sp;

		break;

	case OP_UNE:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].u64 !=
		                                 p_vm->stack[p_vm->sp - 1].u64;
		-- p_vm->sp;

		break;

	case OP_UGR:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].u64 >
		                                 p_vm->stack[p_vm->sp - 1].u64;
		-- p_vm->sp;

		break;

	case OP_UGQ:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].u64 >=
		                                 p_vm->stack[p_vm->sp - 1].u64;
		-- p_vm->sp;

		break;

	case OP_ULE:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].u64 <
		                                 p_vm->stack[p_vm->sp - 1].u64;
		-- p_vm->sp;

		break;

	case OP_ULQ:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].u64 <=
		                                 p_vm->stack[p_vm->sp - 1].u64;
		-- p_vm->sp;

		break;

	case OP_FEQ:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].f64 ==
		                                 p_vm->stack[p_vm->sp - 1].f64;
		-- p_vm->sp;

		break;

	case OP_FNE:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].f64 !=
		                                 p_vm->stack[p_vm->sp - 1].f64;
		-- p_vm->sp;

		break;

	case OP_FGR:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].f64 >
		                                 p_vm->stack[p_vm->sp - 1].f64;
		-- p_vm->sp;

		break;

	case OP_FGQ:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].f64 >=
		                                 p_vm->stack[p_vm->sp - 1].f64;
		-- p_vm->sp;

		break;

	case OP_FLE:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].f64 <
		                                 p_vm->stack[p_vm->sp - 1].f64;
		-- p_vm->sp;

		break;

	case OP_FLQ:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp - 2].u64 = p_vm->stack[p_vm->sp - 2].f64 <=
		                                 p_vm->stack[p_vm->sp - 1].f64;
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

	case OP_DUP:
		if (p_vm->sp >= STACK_CAPACITY)
			return ERR_STACK_OVERFLOW;
		else if (p_vm->sp < inst->data.u64 + 1)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[p_vm->sp] = p_vm->stack[p_vm->sp - inst->data.u64 - 1];

		++ p_vm->sp;

		break;

	case OP_SWP:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;
		else if (p_vm->sp < inst->data.u64 + 2)
			return ERR_STACK_UNDERFLOW;

		word_t tmp = p_vm->stack[p_vm->sp - 1].u64;
		p_vm->stack[p_vm->sp - 1].u64 = p_vm->stack[p_vm->sp - inst->data.u64 - 2].u64;
		p_vm->stack[p_vm->sp - inst->data.u64 - 2].u64 = tmp;

		break;

	case OP_EMP:
		if (p_vm->sp >= STACK_CAPACITY)
			return ERR_STACK_OVERFLOW;

		if (p_vm->sp <= 0)
			p_vm->stack[p_vm->sp ++].u64 = 1;
		else
			p_vm->stack[p_vm->sp ++].u64 = 0;

		break;

	case OP_SET:
		if (p_vm->sp < 3)
			return ERR_STACK_UNDERFLOW;

		{
			word_t  addr = p_vm->stack[p_vm->sp - 3].u64;
			uint8_t val  = p_vm->stack[p_vm->sp - 2].u64;
			word_t  size = p_vm->stack[p_vm->sp - 1].u64;

			if (addr + size >= MEMORY_SIZE_BYTES)
				return ERR_INVALID_MEM_ACCESS;

			p_vm->sp -= 3;

			memset(&p_vm->memory[addr], val, size);
		}

		break;

	case OP_CPY:
		if (p_vm->sp < 3)
			return ERR_STACK_UNDERFLOW;

		{
			word_t to   = p_vm->stack[p_vm->sp - 3].u64;
			word_t from = p_vm->stack[p_vm->sp - 2].u64;
			word_t size = p_vm->stack[p_vm->sp - 1].u64;

			if (to + size >= MEMORY_SIZE_BYTES || from + size >= MEMORY_SIZE_BYTES)
				return ERR_INVALID_MEM_ACCESS;

			p_vm->sp -= 3;

			memcpy(&p_vm->memory[to], &p_vm->memory[from], size);
		}

		break;

	case OP_R08:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		{
			word_t  addr = p_vm->stack[p_vm->sp - 1].u64;
			uint8_t data;
			enum err ret = vm_read8(p_vm, &data, addr);
			if (ret != ERR_OK)
				return ret;

			p_vm->stack[p_vm->sp - 1].u64 = data;
		}

		break;

	case OP_R16:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		{
			word_t   addr = p_vm->stack[p_vm->sp - 1].u64;
			uint16_t data;
			enum err ret = vm_read16(p_vm, &data, addr);
			if (ret != ERR_OK)
				return ret;

			p_vm->stack[p_vm->sp - 1].u64 = data;
		}

		break;

	case OP_R32:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		{
			word_t   addr = p_vm->stack[p_vm->sp - 1].u64;
			uint32_t data;
			enum err ret = vm_read32(p_vm, &data, addr);
			if (ret != ERR_OK)
				return ret;

			p_vm->stack[p_vm->sp - 1].u64 = data;
		}

		break;

	case OP_R64:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		{
			word_t   addr = p_vm->stack[p_vm->sp - 1].u64;
			uint64_t data;
			enum err ret = vm_read64(p_vm, &data, addr);
			if (ret != ERR_OK)
				return ret;

			p_vm->stack[p_vm->sp - 1].u64 = data;
		}

		break;

	case OP_W08:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		{
			word_t  addr = p_vm->stack[p_vm->sp - 2].u64;
			uint8_t data = p_vm->stack[p_vm->sp - 1].u64;
			enum err ret = vm_write8(p_vm, data, addr);
			if (ret != ERR_OK)
				return ret;

			p_vm->sp -= 2;
		}

		break;

	case OP_W16:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		{
			word_t   addr = p_vm->stack[p_vm->sp - 2].u64;
			uint16_t data = p_vm->stack[p_vm->sp - 1].u64;
			enum err ret = vm_write16(p_vm, data, addr);
			if (ret != ERR_OK)
				return ret;

			p_vm->sp -= 2;
		}

		break;

	case OP_W32:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		{
			word_t   addr = p_vm->stack[p_vm->sp - 2].u64;
			uint32_t data = p_vm->stack[p_vm->sp - 1].u64;
			enum err ret = vm_write32(p_vm, data, addr);
			if (ret != ERR_OK)
				return ret;

			p_vm->sp -= 2;
		}

		break;

	case OP_W64:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		{
			word_t   addr = p_vm->stack[p_vm->sp - 2].u64;
			uint64_t data = p_vm->stack[p_vm->sp - 1].u64;
			enum err ret = vm_write64(p_vm, data, addr);
			if (ret != ERR_OK)
				return ret;

			p_vm->sp -= 2;
		}

		break;

	case OP_OPE:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		{
			word_t      addr = p_vm->stack[p_vm->sp - 2].u64;
			enum fmode fmode = (enum fmode)p_vm->stack[p_vm->sp - 1].u64;

			char *name = vm_mem_str_to_cstr(p_vm, addr);
			if (name == NULL)
				return ERR_INVALID_MEM_ACCESS;

			-- p_vm->sp;

			for (word_t i = 0; i < MAX_OPEN_FILES; ++ i) {
				if (p_vm->files[i].file == NULL) {
					char *fmode_str = fmode_to_str(fmode);
					if (fmode_str == NULL)
						return ERR_INVALID_FMODE;

					p_vm->files[i].fmode = fmode;
					p_vm->files[i].file  = fopen(name, fmode_str);
					if (p_vm->files[i].file == NULL)
						p_vm->stack[p_vm->sp - 1].i64 = -1;
					else
						p_vm->stack[p_vm->sp - 1].u64 = i;

					free(fmode_str);
					free(name);

					goto found;
				}
			}

			free(name);

			vm_panic(p_vm, ERR_MAX_FILES_OPEN);
		}

	found:
		break;

	case OP_CLO:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		{
			word_t fd = p_vm->stack[-- p_vm->sp].u64;
			if (fd >= MAX_OPEN_FILES || p_vm->files[fd].file == NULL)
				return ERR_INVALID_FD;

			fclose(p_vm->files[fd].file);
			p_vm->files[fd].file = NULL;
		}

		break;

	case OP_WRF:
		if (p_vm->sp < 2)
			return ERR_STACK_UNDERFLOW;

		{
			word_t  fd   = p_vm->stack[p_vm->sp - 2].u64;
			uint8_t byte = (uint8_t)p_vm->stack[p_vm->sp - 1].u64;

			if (fd >= MAX_OPEN_FILES || p_vm->files[fd].file == NULL)
				return ERR_INVALID_FD;

			fputc((int)byte, p_vm->files[fd].file);

			p_vm->sp -= 2;
		}

		break;

	case OP_RDF:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		{
			word_t fd = p_vm->stack[p_vm->sp - 1].u64;

			if (fd >= MAX_OPEN_FILES || p_vm->files[fd].file == NULL)
				return ERR_INVALID_FD;

			int byte = fgetc(p_vm->files[fd].file);
			p_vm->stack[p_vm->sp - 1].i64 = byte == EOF? -1 : byte;
		}

		break;

	case OP_DMP:
		putchar('\n');
		vm_dump(p_vm, stdout);
		fflush(stdout);

		break;

	case OP_PRT:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		printf("%i\n", (int)p_vm->stack[-- p_vm->sp].i64);

		break;

	case OP_FPR:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		printf("%f\n", (float)p_vm->stack[-- p_vm->sp].f64);

		break;

	case OP_HLT:
		if (p_vm->sp < 1)
			return ERR_STACK_UNDERFLOW;

		p_vm->ex   = p_vm->stack[-- p_vm->sp].u64;
		p_vm->halt = true;

		break;

	default: return ERR_INVALID_INST;
	};

	++ p_vm->ip;

	return ERR_OK;
}

static void dump_inst(FILE *p_file, struct inst *p_inst) {
	if (p_inst == NULL) {
		VM_NOTE(p_file, "End of program");
		return;
	}

	VM_NOTE(p_file, "0x%"FMT_HEX" (%s): 0x%"FMT_HEX" (%f | %lli)", AS_FMT_HEX(p_inst->op),
	        op_to_str[p_inst->op], AS_FMT_HEX(p_inst->data.i64),
	        p_inst->data.f64, (long long)p_inst->data.i64);
}

void vm_debug(struct vm *p_vm) {
#ifdef USES_READLINE
	using_history();
	rl_set_signals();
#else
	char in[256] = {0};
#endif

	while (p_vm->ip < p_vm->program_size && !p_vm->halt) {
#ifdef USES_READLINE
		char *in = readline(PROMPT);
		if (in == NULL) {
			VM_ERROR(stderr, "Failed to read input");
			exit(EXIT_FAILURE);
		}

		add_history(in);
#else
		set_fg_color(COLOR_BRIGHT_BLUE, stdout);
		fputs("(help) ", stdout);
		set_fg_color(COLOR_BRIGHT_MAGENTA, stdout);
		fputs("> ", stdout);
		set_fg_color(COLOR_DEFAULT, stdout);

		if (fgets(in, ARRAY_SIZE(in), stdin) == NULL) {
			VM_ERROR(stderr, "Failed to read input");
			exit(EXIT_FAILURE);
		}
#endif

		char        *cmd  = strtrim(in);
		struct inst *inst = &p_vm->program[p_vm->ip];

		if (strcmp(cmd, "halt") == 0)
			break;
		else if (strcmp(cmd, "help") == 0) {
			VM_NOTE(stdout, "Available commands:");
			puts("  help             Show this message");
			puts("  halt             Quit the program");
			puts("  step (or empty)  Execute the next instruction");
			puts("  regs             Dump the registers");
			puts("  stack            Dump the stack");
			puts("  top              Dump the top of the stack");
			puts("  cstack           Dump the call stack");
			puts("  at               Dump the current instruction position");
			puts("  inst             Dump the current instruction");
			puts("  next             Dump the next instruction");
			puts("  dump             Dump everything");
		} else if (strcmp(cmd, "step") == 0 || strlen(cmd) == 0) { /* continue */
			int ret = vm_exec_next_inst(p_vm);
			if (ret != ERR_OK)
				vm_panic(p_vm, ret);
		} else if (strcmp(cmd, "dump") == 0)
			vm_dump(p_vm, stdout);
		else if (strcmp(cmd, "regs") == 0)
			vm_dump_regs(p_vm, stdout);
		else if (strcmp(cmd, "stack") == 0)
			vm_dump_stack(p_vm, stdout);
		else if (strcmp(cmd, "top") == 0)
			vm_dump_stack_top(p_vm, stdout);
		else if (strcmp(cmd, "cstack") == 0)
			vm_dump_call_stack(p_vm, stdout);
		else if (strcmp(cmd, "at") == 0)
			vm_dump_at(p_vm, stdout);
		else if (strcmp(cmd, "inst") == 0)
			dump_inst(stdout, inst);
		else if (strcmp(cmd, "next") == 0)
			dump_inst(stdout, p_vm->ip + 1 >= p_vm->program_size? NULL : inst + 1);
		else
			VM_ERROR(stderr, "Unknown command '%s'", cmd);

#ifdef USES_READLINE
		free(in);
#endif
	}
}

void vm_run(struct vm *p_vm) {
	while (p_vm->ip < p_vm->program_size && !p_vm->halt) {
		int ret = vm_exec_next_inst(p_vm);
		if (ret != ERR_OK)
			vm_panic(p_vm, ret);
	}
}

void vm_exec_from_mem(struct vm *p_vm, struct inst *p_program, word_t p_program_size, word_t p_ep) {
	p_vm->program      = p_program;
	p_vm->program_size = p_program_size;
	p_vm->ip           = p_ep;

	if (p_vm->debug)
		vm_debug(p_vm);
	else
		vm_run(p_vm);
}

static word_t bytes_to_word(uint8_t *p_bytes) {
	return (word_t)p_bytes[0] << 070 |
	       (word_t)p_bytes[1] << 060 |
	       (word_t)p_bytes[2] << 050 |
	       (word_t)p_bytes[3] << 040 |
	       (word_t)p_bytes[4] << 030 |
	       (word_t)p_bytes[5] << 020 |
	       (word_t)p_bytes[6] << 010 |
	       (word_t)p_bytes[7];
}

void vm_exec_from_file(struct vm *p_vm, const char *p_path) {
	FILE *file = fopen(p_path, "rb");
	if (file == NULL) {
		VM_ERROR(stderr, "Failed to open file '%s': %s", p_path, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* skip the shebang */
	char ch = fgetc(file);
	if (ch == '#')
		while (fgetc(file) != '\n');
	else
		ungetc(ch, file);

	struct file_meta meta;
	size_t ret = fread(&meta, sizeof(meta), 1, file);
	if (ret < 1) {
		VM_ERROR(stderr, "Error while reading '%s' metadata: %s", p_path, strerror(errno));
		exit(EXIT_FAILURE);
	}

	assert(sizeof(meta.magic) == 3);
	if (strncmp(meta.magic, "AVM", 3) != 0) {
		VM_ERROR(stderr, "'%s' is not an executable AVM binary", p_path);
		exit(EXIT_FAILURE);
	}

	if (meta.ver[0] != VERSION_MAJOR && p_vm->warnings)
		VM_WARN(stderr, "'%s' major version is %i, your avm major version is %i",
		        p_path, meta.ver[0], VERSION_MAJOR);
	else if (meta.ver[1] > VERSION_MINOR && p_vm->warnings)
		VM_WARN(stderr, "'%s' minor version is %i, greater than your avm minor version which is %i",
		        p_path, meta.ver[1], VERSION_MINOR);

	// Ignore the patch version
	// else if (meta.ver[2] != VERSION_PATCH)
	// 	warning("'%s' patch version is %i, your VM patch version is %i",
	// 	        p_path, meta.ver[2], VERSION_PATCH);

	word_t program_size = bytes_to_word(meta.program_size);
	word_t memory_size  = bytes_to_word(meta.memory_size);
	word_t entry_point  = bytes_to_word(meta.entry_point);

	for (word_t i = 0; i < memory_size; ++ i) {
		if (i >= MEMORY_SIZE_BYTES) {
			VM_ERROR(stderr, "'%s' memory segment is bigger than VM memory (%zu bytes)",
			         p_path, i + 1);
			exit(EXIT_FAILURE);
		}

		int byte = fgetc(file);
		if (byte == EOF) {
			VM_ERROR(stderr, "'%s' unexpected EOF during memory segment", p_path);
			exit(EXIT_FAILURE);
		}

		p_vm->memory[i + 1] = byte;
	}

	struct inst *program = (struct inst*)malloc(sizeof(struct inst) * program_size);
	if (program == NULL) {
		VM_ERROR(stderr, "malloc() fail");
		exit(EXIT_FAILURE);
	}

	for (word_t i = 0; i < program_size; ++ i) {
		uint8_t inst[sizeof(struct inst)];

		ret = fread(&inst, sizeof(inst), 1, file);
		if (ret < 1) {
			VM_ERROR(stderr, "'%s' incompatible instruction format at instruction %zu",
			         p_path, i + 1);
			exit(EXIT_FAILURE);
		}

		program[i].op       = (enum opcode)inst[0];
		program[i].data.u64 = bytes_to_word(inst + 1);
	}

	fclose(file);

	vm_exec_from_mem(p_vm, program, program_size, entry_point);

	free(program);
}
