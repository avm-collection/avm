#ifndef VM_H__HEADER_GUARD__
#define VM_H__HEADER_GUARD__

#include <stdint.h>  /* uint64_t, uint8_t */
#include <string.h>  /* memset, memcpy, strncmp, strcmp, strlen */
#include <stdio.h>   /* stderr, fputs, fputc, putchar, fprintf, FILE, fflush,
                        fopen, fclose, fread, ftell, fseek */
#include <stdbool.h> /* bool, true, false */
#include <stdlib.h>  /* exit, malloc, free, EXIT_FAILURE */
#include <assert.h>  /* static_assert */
#include <dlfcn.h>   /* dlopen, dlclose, dlsym */

#include "config.h"
#include "platform.h"
#include "utils.h"
#include "color.h"

#ifdef PLATFORM_LINUX
#	define EXPORT
#else
#	define EXPORT __declspec(dllexport)
#endif

#define AVM_BINDING(P_NAME, P_VM) enum err avm_##P_NAME(struct vm *P_VM)

#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#	define PACK(P_STRUCT) P_STRUCT __attribute__((__packed__))
#elif defined(COMPILER_MSVC)
#	define PACK(P_STRUCT) __pragma(pack(push, 1)) P_STRUCT __pragma(pack(pop))
#else
#	define PACK(P_STRUCT) P_STRUCT __attribute__((__packed__))
#endif

#define STACK_SIZE_BYTES 0x10000
#define STACK_CAPACITY   (STACK_SIZE_BYTES / sizeof(value_t))

#define CALL_STACK_SIZE_BYTES 0x1000
#define CALL_STACK_CAPACITY   (CALL_STACK_SIZE_BYTES / sizeof(word_t))

#define MAX_OPEN_FILES   0x100
#define MAX_OPEN_LIBS    0x80
#define MAX_LOADED_FUNCS 0x80

#define INVALID_DESCRIPTOR (word_t)-1

#define FMT_HEX         "016llX"
#define AS_FMT_HEX(P_X) (long long unsigned)(P_X)

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

	/* Push, pop */
	OP_PSH = 0x10,
	OP_POP = 0x11,

	/* Arithmetic */
	OP_ADD = 0x20,
	OP_SUB = 0x21,

	OP_MUL = 0x22,
	OP_DIV = 0x23,
	OP_MOD = 0x24,

	OP_INC = 0x25,
	OP_DEC = 0x26,

	/* Float arithmetic */
	OP_FAD = 0x27,
	OP_FSB = 0x28,

	OP_FMU = 0x29,
	OP_FDI = 0x2a,

	OP_FIN = 0x2b,
	OP_FDE = 0x2c,

	OP_NEG = 0x2d,
	OP_NOT = 0x2e,

	/* Logic */
	OP_JMP = 0x30,
	OP_JNZ = 0x31,

	OP_CAL = 0x38,
	OP_RET = 0x39,

	OP_AND = 0x46,
	OP_ORR = 0x47,

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
	OP_SET = 0x53,
	OP_CPY = 0x54,

	/* Memory */
	OP_R08 = 0x60,
	OP_R16 = 0x61,
	OP_R32 = 0x62,
	OP_R64 = 0x63,

	OP_W08 = 0x64,
	OP_W16 = 0x65,
	OP_W32 = 0x66,
	OP_W64 = 0x67,

	/* File IO */
	OP_OPE = 0x70,
	OP_CLO = 0x71,
	OP_WRF = 0x72,
	OP_RDF = 0x73,
	OP_SZF = 0x74,
	OP_FLU = 0x75,

	/* Bit arithmetic */
	OP_BAN = 0x80,
	OP_BOR = 0x81,
	OP_BSR = 0x82,
	OP_BSL = 0x83,

	/* Shared library */
	OP_LOL = 0x90,
	OP_CLL = 0x91,
	OP_LLF = 0x92,
	OP_ULF = 0x93,
	OP_CLF = 0x94,

	/* Debug */
	OP_DMP = 0xF0,
	OP_PRT = 0xF1,
	OP_FPR = 0xF2,

	OP_HLT = 0xFF,
};

enum err {
	ERR_OK                   = 0x00,
	ERR_STACK_OVERFLOW       = 0x01,
	ERR_STACK_UNDERFLOW      = 0x02,
	ERR_CALL_STACK_OVERFLOW  = 0x03,
	ERR_CALL_STACK_UNDERFLOW = 0x04,
	ERR_INVALID_INST         = 0x05,
	ERR_INVALID_INST_ACCESS  = 0x06,
	ERR_INVALID_MEM_ACCESS   = 0x07,
	ERR_DIV_BY_ZERO          = 0x08,
	ERR_MAX_FILES_OPEN       = 0x09,
	ERR_INVALID_FMODE        = 0x0a,
	ERR_INVALID_DESCRIPTOR   = 0x0b,
	ERR_MAX_LIBS_OPEN        = 0x0d,
	ERR_MAX_FUNCS_LOADED     = 0x0e,
};

const char *err_str(enum err p_err);

enum fmode {
	FMODE_READ   = 1 << 0,
	FMODE_WRITE  = 1 << 1,
	FMODE_APPEND = 1 << 2,
	FMODE_BINARY = 1 << 3,
};

char *fmode_to_str(enum fmode p_fmode);

struct file {
	FILE      *file;
	enum fmode mode;
};

struct vm;
typedef enum err (*external_t)(struct vm*);

struct lib {
	void      *handle;
	external_t funcs[MAX_LOADED_FUNCS];
};

PACK(struct inst {
	enum opcode op: 8;
	value_t     data;
});

struct maps {
	struct file files[MAX_OPEN_FILES];
	struct lib  libs[MAX_OPEN_LIBS];
};

struct vm {
	value_t *stack;
	word_t  *call_stack;
	word_t   ip, sp, cs, ex; /* Registers */
	uint8_t *memory;
	word_t   memory_size;

	struct maps *maps;

	struct inst *program;
	word_t       program_size;

	bool halt;
};

PACK(struct file_meta {
	char    magic[3]; /* AVM */
	uint8_t ver[3];   /* [0] = MAJOR, [1] = MINOR, [2] = PATCH */
	uint8_t program_size[sizeof(word_t)];
	uint8_t memory_size[sizeof(word_t)];
	uint8_t entry_point[sizeof(word_t)];
});

void vm_init(struct vm *p_vm);
void vm_alloc_mem(struct vm *p_vm, word_t p_bytes);
void vm_destroy(struct vm *p_vm);

int  vm_exec_next_inst(struct vm *p_vm);
void vm_load_from_mem(struct vm *p_vm, struct inst *p_program, word_t p_size, word_t p_ep);
void vm_run(struct vm *p_vm);

void vm_dump(struct vm *p_vm, FILE *p_file);
void vm_dump_regs(struct vm *p_vm, FILE *p_file);
void vm_dump_stack_top(struct vm *p_vm, FILE *p_file);
void vm_dump_stack(struct vm *p_vm, FILE *p_file);
void vm_dump_call_stack(struct vm *p_vm, FILE *p_file);
void vm_dump_at(struct vm *p_vm, FILE *p_file);
void vm_dump_inst(struct vm *p_vm, FILE *p_file);

void vm_panic(struct vm *p_vm, enum err p_err);

void log_colored(FILE *p_file, enum color p_color, const char *p_fmt, ...);

#define VM_ERROR(P_FILE, ...) log_colored(P_FILE, COLOR_BRIGHT_RED,    __VA_ARGS__)
#define VM_WARN(P_FILE,  ...) log_colored(P_FILE, COLOR_BRIGHT_YELLOW, __VA_ARGS__)
#define VM_NOTE(P_FILE,  ...) log_colored(P_FILE, COLOR_BRIGHT_CYAN,   __VA_ARGS__)

enum err vm_read8 (struct vm *p_vm, uint8_t  *p_data, word_t p_addr);
enum err vm_read16(struct vm *p_vm, uint16_t *p_data, word_t p_addr);
enum err vm_read32(struct vm *p_vm, uint32_t *p_data, word_t p_addr);
enum err vm_read64(struct vm *p_vm, uint64_t *p_data, word_t p_addr);

enum err vm_write8 (struct vm *p_vm, uint8_t  p_data, word_t p_addr);
enum err vm_write16(struct vm *p_vm, uint16_t p_data, word_t p_addr);
enum err vm_write32(struct vm *p_vm, uint32_t p_data, word_t p_addr);
enum err vm_write64(struct vm *p_vm, uint64_t p_data, word_t p_addr);

word_t vm_get_free_fd(struct vm *p_vm);
word_t vm_get_free_ld(struct vm *p_vm);
word_t vm_get_free_fnd(struct vm *p_vm, word_t p_ld);

bool vm_get_str(struct vm *p_vm, char *p_buf, word_t p_addr, word_t p_size);

bool vm_is_fd_valid(struct vm *p_vm, word_t p_fd);
bool vm_is_ld_valid(struct vm *p_vm, word_t p_ld);
bool vm_is_fnd_valid(struct vm *p_vm, word_t p_ld, word_t p_fnd);
bool vm_is_chunk_valid(struct vm *p_vm, word_t p_addr, word_t p_size);

value_t *vm_stack_top(struct vm *p_vm, word_t p_off);

#endif
