#ifndef VM_H__HEADER_GUARD__
#define VM_H__HEADER_GUARD__

#include <stdint.h>  /* uint64_t, uint8_t */
#include <string.h>  /* memset, strncmp */
#include <stdio.h>   /* stderr, fputs, fputc, fprintf, FILE, fflush, fopen, fclose, fread */
#include <stdbool.h> /* bool, true, false */
#include <stdlib.h>  /* exit, malloc, free */

#include "utils.h"

#define STACK_CAPACITY 0xFFFF

typedef uint64_t word_t;

enum {
	OP_NOP = 0,

	OP_MOV,
	OP_MOV_REG,

	OP_PUSH,
	OP_PUSH_REG,
	OP_POP,
	OP_POP_REG,

	OP_ADD,
	OP_SUB,

	OP_MUL,
	OP_DIV,
	OP_MOD,

	OP_DUMP,
	OP_HALT = 0xFF
};

enum {
	REG_1 = 0,
	REG_2,
	REG_3,
	REG_4,
	REG_5,
	REG_6,
	REG_7,
	REG_8,
	REG_9,
	REG_10,
	REG_11,
	REG_12,
	REG_13,
	REG_14,
	REG_15,
	REG_16,

	REG_AC, /* accumulator */
	REG_IP, /* instruction pointer */
	REG_SP, /* stack pointer */
	REG_SB, /* stack base pointer */
	REG_CN, /* conditional */
	REG_EX, /* exitcode */

	REGS_COUNT
};

enum {
	ERR_OK = 0,
	ERR_STACK_OVERFLOW,
	ERR_STACK_UNDERFLOW,
	ERR_ILLEGAL_INST,
	ERR_INVALID_ACCESS,
	ERR_DIV_BY_ZERO
};

struct inst {
	uint8_t op, reg;
	word_t  data;
};

struct vm {
	word_t stack[STACK_CAPACITY];
	word_t regs[REGS_COUNT];

	word_t *ac, *ip, *sp, *sb, *cn, *ex;

	int  err;
	bool halt;
};

PACK(struct file_meta {
	char    magic[3]; /* AVM */
	uint8_t ver[3];   /* [0] = MAJOR, [1] = MINOR, [2] = PATCH */
	uint8_t program_size[sizeof(word_t)];
	uint8_t entry_point[sizeof(word_t)];
});

void vm_dump(struct vm *p_vm, FILE *p_file);

void vm_exec(struct vm *p_vm, struct inst *p_program, word_t p_size, word_t p_ep);
void vm_exec_from_file(struct vm *p_vm, const char *p_path);

#endif
