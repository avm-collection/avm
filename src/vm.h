#ifndef VM_H__HEADER_GUARD__
#define VM_H__HEADER_GUARD__

#include <stdint.h>  /* uint64_t, uint8_t */
#include <string.h>  /* memset, strncmp */
#include <stdio.h>   /* stderr, fputs, fputc, fprintf, FILE, fflush, fopen, fclose, fread */
#include <stdbool.h> /* bool, true, false */
#include <stdlib.h>  /* exit, malloc, free */
#include <errno.h>   /* strerror, errno */

#include "utils.h"

#define STACK_SIZE_BYTES 0x10000
#define STACK_CAPACITY   (STACK_SIZE_BYTES / sizeof(word_t))

typedef uint64_t word_t;

enum opcode {
	OP_NOP = 0,

	OP_MOV,
	OP_MOR,

	OP_PSH,
	OP_PSR,
	OP_POP,
	OP_POR,

	OP_ADD,
	OP_SUB,

	OP_MUL,
	OP_DIV,
	OP_MOD,

	OP_INC,
	OP_DEC,

	OP_JMP,
	OP_JNZ,

	OP_EQU,
	OP_NEQ,
	OP_GRT,
	OP_GEQ,
	OP_LES,
	OP_LEQ,

	OP_DUP,
	OP_SWP,

	OP_DUM = 0xFE,
	OP_HLT = 0xFF
};

enum reg {
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

	REG_IP, /* instruction pointer */
	REG_SP, /* stack pointer */
	REG_SB, /* stack base pointer */
	REG_EX, /* exitcode */

	REGS_COUNT
};

enum err {
	ERR_OK = 0,
	ERR_STACK_OVERFLOW,
	ERR_STACK_UNDERFLOW,
	ERR_ILLEGAL_INST,
	ERR_INVALID_ACCESS,
	ERR_DIV_BY_ZERO
};

const char *err_str(enum err p_err);

struct inst {
	enum opcode op:  8;
	enum reg    reg: 8;
	word_t      data;
};

struct vm {
	word_t stack[STACK_CAPACITY];
	word_t regs[REGS_COUNT];

	word_t *ip, *sp, *sb, *ex;

	struct inst *program;
	word_t       program_size;

	bool halt;
};

PACK(struct file_meta {
	char    magic[3]; /* AVM */
	uint8_t ver[3];   /* [0] = MAJOR, [1] = MINOR, [2] = PATCH */
	uint8_t program_size[sizeof(word_t)];
	uint8_t entry_point[sizeof(word_t)];
});

void vm_dump(struct vm *p_vm, FILE *p_file);

void vm_exec_from_mem(struct vm *p_vm, struct inst *p_program, word_t p_program_size, word_t p_ep);
void vm_exec_from_file(struct vm *p_vm, const char *p_path);

#endif
