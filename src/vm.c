#include "vm.h"
#include "main.h"

const char *err_str(enum err p_err) {
	switch (p_err) {
	case ERR_OK: return "OK";
	case ERR_STACK_OVERFLOW:  return "Stack Overflow";
	case ERR_STACK_UNDERFLOW: return "Stack Underflow";
	case ERR_ILLEGAL_INST:    return "Illegal Instruction";
	case ERR_INVALID_ACCESS:  return "Invalid Access";
	case ERR_DIV_BY_ZERO:     return "Division By Zero";

	default: UNREACHABLE();
	}
}

void vm_dump(struct vm *p_vm, FILE *p_file) {
	assert(sizeof(word_t) == 8);

	fputs("  REGS\n", p_file);

	for (int i = 0; i < REGS_COUNT; ++ i) {
		fprintf(p_file, "%016llX ", (long long unsigned)p_vm->regs[i]);

		if ((i + 1) % 2 == 0)
			fputc('\n', p_file);
	}

	fputs("\n  STACK\n", p_file);

	if (*p_vm->sp <= *p_vm->sb)
		fputs("EMPTY\n", p_file);
	else {
		int c = 0;
		for (word_t i = *p_vm->sb; i < *p_vm->sp; ++ i) {
			fprintf(p_file, "%016llX ", (long long unsigned)p_vm->stack[i]);

			if ((c + 1) % 4 == 0)
				fputc('\n', p_file);

			++ c;
		}

		if ((c + 1) % 4 != 0)
			fputc('\n', p_file);
	}

	fflush(p_file);
}

static word_t *vm_reg(struct vm *p_vm, uint8_t p_reg) {
	if (p_reg > REGS_COUNT)
		return NULL;

	return &p_vm->regs[p_reg];
}

static int vm_exec_next_inst(struct vm *p_vm) {
	struct inst *inst = &p_vm->program[*p_vm->ip];

	switch (inst->op) {
	case OP_NOP: break;

	case OP_MOV:
		{
			word_t *reg = vm_reg(p_vm, inst->reg);
			if (reg == NULL)
				return ERR_INVALID_ACCESS;

			*reg = inst->data;
		}

		break;

	case OP_MOR:
		{
			word_t *reg_a = vm_reg(p_vm, inst->reg);
			if (reg_a == NULL)
				return ERR_INVALID_ACCESS;

			word_t *reg_b = vm_reg(p_vm, (uint8_t)inst->data);
			if (reg_b == NULL)
				return ERR_INVALID_ACCESS;

			*reg_a = *reg_b;
		}

		break;

	case OP_PSH:
		if (*p_vm->sp >= STACK_CAPACITY)
			return ERR_STACK_OVERFLOW;

		p_vm->stack[(*p_vm->sp) ++] = inst->data;

		break;

	case OP_PSR:
		if (*p_vm->sp >= STACK_CAPACITY)
			return ERR_STACK_OVERFLOW;

		{
			word_t *reg = vm_reg(p_vm, inst->reg);
			if (reg == NULL)
				return ERR_INVALID_ACCESS;

			p_vm->stack[(*p_vm->sp) ++] = *reg;
		}

		break;

	case OP_POP:
		if (*p_vm->sp <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		-- *p_vm->sp;

		break;

	case OP_POR:
		if (*p_vm->sp <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		{
			word_t *reg = vm_reg(p_vm, inst->reg);
			if (reg == NULL)
				return ERR_INVALID_ACCESS;

			*reg = p_vm->stack[-- (*p_vm->sp)];
		}

		break;

	case OP_ADD:
		if (*p_vm->sp - 1 <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[*p_vm->sp - 2] += p_vm->stack[*p_vm->sp - 1];
		-- *p_vm->sp;

		break;

	case OP_SUB:
		if (*p_vm->sp - 1 <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[*p_vm->sp - 2] -= p_vm->stack[*p_vm->sp - 1];
		-- *p_vm->sp;

		break;

	case OP_MUL:
		if (*p_vm->sp - 1 <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[*p_vm->sp - 2] *= p_vm->stack[*p_vm->sp - 1];
		-- *p_vm->sp;

		break;

	case OP_DIV:
		if (*p_vm->sp - 1 <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		{
			word_t b = p_vm->stack[*p_vm->sp - 1];
			if (b == 0)
				return ERR_DIV_BY_ZERO;

			p_vm->stack[*p_vm->sp - 2] /= b;
			-- *p_vm->sp;
		}

		break;

	case OP_MOD:
		if (*p_vm->sp - 1 <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[*p_vm->sp - 2] %= p_vm->stack[*p_vm->sp - 1];
		-- *p_vm->sp;

		break;

	case OP_INC:
		if (*p_vm->sp <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		++ p_vm->stack[*p_vm->sp - 1];

		break;

	case OP_DEC:
		if (*p_vm->sp <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		-- p_vm->stack[*p_vm->sp - 1];

		break;

	case OP_JMP:
		if (inst->data >= p_vm->program_size)
			return ERR_INVALID_ACCESS;

		*p_vm->ip = inst->data - 1;

		break;

	case OP_JNZ:
		if (*p_vm->sp <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		if (p_vm->stack[*p_vm->sp - 1]) {
			if (inst->data >= p_vm->program_size)
				return ERR_INVALID_ACCESS;

			*p_vm->ip = inst->data - 1;
		}

		-- *p_vm->sp;

		break;

	case OP_EQU:
		if (*p_vm->sp - 1 <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[*p_vm->sp - 2] = p_vm->stack[*p_vm->sp - 2] == p_vm->stack[*p_vm->sp - 1];
		-- *p_vm->sp;

		break;

	case OP_NEQ:
		if (*p_vm->sp - 1 <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[*p_vm->sp - 2] = p_vm->stack[*p_vm->sp - 2] != p_vm->stack[*p_vm->sp - 1];
		-- *p_vm->sp;

		break;

	case OP_GRT:
		if (*p_vm->sp - 1 <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[*p_vm->sp - 2] = p_vm->stack[*p_vm->sp - 2] > p_vm->stack[*p_vm->sp - 1];
		-- *p_vm->sp;

		break;

	case OP_GEQ:
		if (*p_vm->sp - 1 <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[*p_vm->sp - 2] = p_vm->stack[*p_vm->sp - 2] >= p_vm->stack[*p_vm->sp - 1];
		-- *p_vm->sp;

		break;

	case OP_LES:
		if (*p_vm->sp - 1 <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[*p_vm->sp - 2] = p_vm->stack[*p_vm->sp - 2] < p_vm->stack[*p_vm->sp - 1];
		-- *p_vm->sp;

		break;

	case OP_LEQ:
		if (*p_vm->sp - 1 <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		p_vm->stack[*p_vm->sp - 2] = p_vm->stack[*p_vm->sp - 2] <= p_vm->stack[*p_vm->sp - 1];
		-- *p_vm->sp;

		break;

	case OP_DUP:
		if (*p_vm->sp >= STACK_CAPACITY)
			return ERR_STACK_OVERFLOW;

		p_vm->stack[*p_vm->sp] = p_vm->stack[*p_vm->sp - 1];

		++ *p_vm->sp;

		break;

	case OP_SWP:
		if (*p_vm->sp - 1 <= *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		word_t tmp = p_vm->stack[*p_vm->sp - 1];
		p_vm->stack[*p_vm->sp - 1] = p_vm->stack[*p_vm->sp - 1];
		p_vm->stack[*p_vm->sp - 2] = tmp;

		break;

	case OP_DUM: vm_dump(p_vm, stdout); break;
	case OP_HLT: p_vm->halt = true;     break;

	default: return ERR_ILLEGAL_INST;
	};

	++ *p_vm->ip;

	return ERR_OK;
}

void vm_exec_from_mem(struct vm *p_vm, struct inst *p_program, word_t p_program_size, word_t p_ep) {
	memset(p_vm, 0, sizeof(struct vm));

	assert(STACK_SIZE_BYTES % sizeof(word_t) == 0);

	p_vm->ac = &p_vm->regs[REG_AC];
	p_vm->ip = &p_vm->regs[REG_IP];
	p_vm->sp = &p_vm->regs[REG_SP];
	p_vm->sb = &p_vm->regs[REG_SB];
	p_vm->cn = &p_vm->regs[REG_CN];
	p_vm->ex = &p_vm->regs[REG_EX];

	p_vm->program      = p_program;
	p_vm->program_size = p_program_size;

	for (*p_vm->ip = p_ep; *p_vm->ip < p_vm->program_size && !p_vm->halt;) {
		int ret = vm_exec_next_inst(p_vm);
		if (ret != ERR_OK) {
			fprintf(stderr, "%s (0x%016llX)\n", err_str(ret), (unsigned long long)*p_vm->ip);

			exit(ret);
		}
	}
}

static word_t bytes_to_word(uint8_t *p_bytes) {
	assert(sizeof(word_t) == 8);

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
	FILE *file = fopen(p_path, "r");
	if (file == NULL)
		fatal("File '%s' not found", p_path);

	/* skip the shebang */
	char ch = fgetc(file);
	if (ch == '#')
		while (fgetc(file) != '\n');
	else
		ungetc(ch, file);

	struct file_meta meta;
	size_t ret = fread(&meta, sizeof(meta), 1, file);
	if (ret < 1)
		fatal("Error while reading '%s' metadata", p_path);

	assert(sizeof(meta.magic) == 3);
	if (strncmp(meta.magic, "AVM", 3) != 0)
		fatal("'%s' is not in an executable AVM format", p_path);

	if (meta.ver[0] != VERSION_MAJOR)
		fatal("'%s' version is %i, expected %i", meta.ver[0], VERSION_MAJOR);
	else if (meta.ver[1] != VERSION_MINOR)
		fatal("'%s' version is %i, expected %i", meta.ver[1], VERSION_MINOR);
	else if (meta.ver[2] != VERSION_PATCH)
		fatal("'%s' version is %i, expected %i", meta.ver[2], VERSION_PATCH);

	word_t size = bytes_to_word(meta.program_size);
	word_t ep   = bytes_to_word(meta.entry_point);

	struct inst *program = (struct inst*)malloc(sizeof(struct inst) * size);
	if (program == NULL)
		assert(0 && "malloc fail");

	for (size_t i = 0; i < size; ++ i) {
		uint8_t inst[sizeof(word_t) + 2];

		ret = fread(&inst, sizeof(inst), 1, file);
		if (ret < 1)
			fatal("Error while reading '%s' instructions", p_path);

		program[i].op   = inst[0];
		program[i].reg  = inst[1];
		program[i].data = bytes_to_word(inst + 2);
	}

	fclose(file);

	vm_exec_from_mem(p_vm, program, size, ep);

	free(program);
}
