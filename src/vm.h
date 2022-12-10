#ifndef VM_H__HEADER_GUARD__
#define VM_H__HEADER_GUARD__

#include <stdint.h>  /* uint64_t, uint8_t */
#include <string.h>  /* memset, strncmp */
#include <stdio.h>   /* stderr, fputs, fputc, fprintf, FILE, fflush, fopen, fclose, fread */
#include <stdbool.h> /* bool, true, false */
#include <stdlib.h>  /* exit, malloc, free */
#include <errno.h>   /* strerror, errno */

#include "config.h"
#include "utils.h"

#if defined(__COMPILER_GCC__) || defined(__COMPILER_CLANG__)
#	define PACK(p_struct) p_struct __attribute__((__packed__))
#elif defined(__COMPILER_MSVC__)
#	define PACK(p_struct) __pragma(pack(push, 1)) p_struct __pragma(pack(pop))
#else
#	define PACK(p_struct) p_struct __attribute__((__packed__))
#endif

#define STACK_SIZE_BYTES 0x10000
#define STACK_CAPACITY   (STACK_SIZE_BYTES / sizeof(word_t))

typedef uint64_t word_t;

typedef union {
	uint64_t u64;
	int64_t  i64;
	double   f64;
	void    *ptr;
} value_t;

/* Asserts */
static_assert(STACK_SIZE_BYTES % sizeof(word_t) == 0);
static_assert(sizeof(word_t)  == 8);
static_assert(sizeof(double)  == sizeof(word_t));
static_assert(sizeof(value_t) == sizeof(word_t));

enum opcode {
	OP_NOP = 0x00,

	/* Move */
	OP_MOV = 0x10,
	OP_MOR = 0x11,

	/* Push, pop */
	OP_PSH = 0x12,
	OP_PSR = 0x13,
	OP_POP = 0x14,
	OP_POR = 0x15,

	/* Arithmetic */
	OP_ADD = 0x20,
	OP_SUB = 0x21,

	OP_MUL = 0x22,
	OP_DIV = 0x23,
	OP_MOD = 0x24,

	OP_DEC = 0x25,
	OP_INC = 0x26,

	/* Float arithmetic */
	OP_FAD = 0x27,
	OP_FSB = 0x28,

	OP_FMU = 0x29,
	OP_FDI = 0x2a,

	OP_FIN = 0x2b,
	OP_FDE = 0x2c,

	/* Logic */
	OP_JMP = 0x30,
	OP_JNZ = 0x31,

	OP_CAL = 0x38,
	OP_RET = 0x39,

	/* Signed comparisons */
	OP_EQU = 0x32,
	OP_NEQ = 0x33,
	OP_GRT = 0x34,
	OP_GEQ = 0x35,
	OP_LES = 0x36,
	OP_LEQ = 0x37,

	/* Unsigned comparisons */
	OP_UEQ = 0x3a,
	OP_UNE = 0x3b,
	OP_UGR = 0x3c,
	OP_UGQ = 0x3d,
	OP_ULE = 0x3e,
	OP_ULQ = 0x3f,

	/* Float comparisons */
	OP_FEQ = 0x40,
	OP_FNE = 0x41,
	OP_FGR = 0x42,
	OP_FGQ = 0x43,
	OP_FLE = 0x44,
	OP_FLQ = 0x45,

	/* Misc */
	OP_DUP = 0x50,
	OP_SWP = 0x51,
	OP_EMP = 0x52,

	/* Debug */
	OP_DMP = 0xF0,
	OP_PRT = 0xF1,
	OP_FPR = 0xF2,

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

	REG_IP, /* Instruction pointer */
	REG_SP, /* Stack pointer */
	REG_SB, /* Stack base pointer */
	REG_EX, /* Exitcode */

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
	value_t     data;
};

struct vm {
	value_t stack[STACK_CAPACITY];
	word_t  regs[REGS_COUNT];

	word_t *ip, *sp, *sb, *ex;

	struct inst *program;
	word_t       program_size;

	bool halt, warnings;
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
