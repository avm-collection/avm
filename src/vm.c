#include "vm.h"
#include "main.h"

static const char *err_str(int p_err) {
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

static int vm_exec_inst(struct vm *p_vm, struct inst p_inst) {
	switch (p_inst.op) {
	case OP_NOP: break;

	case OP_MOV:
		{
			word_t *reg = vm_reg(p_vm, p_inst.reg);
			if (reg == NULL)
				return ERR_INVALID_ACCESS;

			*reg = p_inst.data;
		}

		break;

	case OP_MOV_REG:
		{
			word_t *reg_a = vm_reg(p_vm, p_inst.reg);
			if (reg_a == NULL)
				return ERR_INVALID_ACCESS;

			word_t *reg_b = vm_reg(p_vm, (uint8_t)p_inst.data);
			if (reg_b == NULL)
				return ERR_INVALID_ACCESS;

			*reg_a = *reg_b;
		}

		break;

	case OP_PUSH:
		if (*p_vm->sp + 1 > STACK_CAPACITY)
			return ERR_STACK_OVERFLOW;

		p_vm->stack[(*p_vm->sp) ++] = p_inst.data;

		break;

	case OP_PUSH_REG:
		if (*p_vm->sp + 1 > STACK_CAPACITY)
			return ERR_STACK_OVERFLOW;

		{
			word_t *reg = vm_reg(p_vm, p_inst.reg);
			if (reg == NULL)
				return ERR_INVALID_ACCESS;

			p_vm->stack[(*p_vm->sp) ++] = *reg;
		}

		break;

	case OP_POP:
		-- *p_vm->sp;

		if (*p_vm->sp < *p_vm->sb)
			return ERR_STACK_UNDERFLOW;

		break;

	case OP_POP_REG:
		{
			word_t *reg = vm_reg(p_vm, p_inst.reg);
			if (reg == NULL)
				return ERR_INVALID_ACCESS;

			*reg = p_vm->stack[(*p_vm->sp) --];

			if (*p_vm->sp < *p_vm->sb)
				return ERR_STACK_UNDERFLOW;
		}

		break;

	case OP_ADD:
		{
			word_t a = p_vm->stack[*p_vm->sp - 2], b = p_vm->stack[*p_vm->sp - 1];

			-- *p_vm->sp;
			p_vm->stack[*p_vm->sp - 1] = a + b;
		}

		break;

	case OP_SUB:
		{
			word_t a = p_vm->stack[*p_vm->sp - 2], b = p_vm->stack[*p_vm->sp - 1];

			-- *p_vm->sp;
			p_vm->stack[*p_vm->sp - 1] = a - b;
		}

		break;

	case OP_MUL:
		{
			word_t a = p_vm->stack[*p_vm->sp - 2], b = p_vm->stack[*p_vm->sp - 1];

			-- *p_vm->sp;
			p_vm->stack[*p_vm->sp - 1] = a * b;
		}

		break;

	case OP_DIV:
		{
			word_t a = p_vm->stack[*p_vm->sp - 2], b = p_vm->stack[*p_vm->sp - 1];

			if (b == 0)
				return ERR_DIV_BY_ZERO;

			*p_vm->ac = a % b;

			-- *p_vm->sp;
			p_vm->stack[*p_vm->sp - 1] = a / b;
		}

		break;

	case OP_MOD:
		{
			word_t a = p_vm->stack[*p_vm->sp - 2], b = p_vm->stack[*p_vm->sp - 1];

			-- *p_vm->sp;
			p_vm->stack[*p_vm->sp - 1] = a % b;
		}

		break;

	case OP_DUMP: vm_dump(p_vm, stdout); break;
	case OP_HALT: p_vm->halt = true;     break;

	default: return ERR_ILLEGAL_INST;
	};

	return ERR_OK;
}

void vm_exec(struct vm *p_vm, struct inst *p_program, word_t p_size, word_t p_ep) {
	memset(p_vm, 0, sizeof(struct vm));

	p_vm->ac = &p_vm->regs[REG_AC];
	p_vm->ip = &p_vm->regs[REG_IP];
	p_vm->sp = &p_vm->regs[REG_SP];
	p_vm->sb = &p_vm->regs[REG_SB];
	p_vm->cn = &p_vm->regs[REG_CN];
	p_vm->ex = &p_vm->regs[REG_EX];

	for (*p_vm->ip = p_ep; *p_vm->ip < p_size && !p_vm->halt; ++ *p_vm->ip) {
		int ret = vm_exec_inst(p_vm, p_program[*p_vm->ip]);
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

	vm_exec(p_vm, program, size, ep);

	free(program);
}
