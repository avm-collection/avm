#include "debugger.h"

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

	[OP_AND] = "AND",
	[OP_ORR] = "ORR",

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
	[OP_FLU] = "FLU",

	[OP_BAN] = "BAN",
	[OP_BOR] = "BOR",
	[OP_BSR] = "BSR",
	[OP_BSL] = "BSL",

	[OP_LOL] = "LOL",
	[OP_CLL] = "CLL",
	[OP_LLF] = "LLF",
	[OP_ULF] = "ULF",
	[OP_CLF] = "CLF",

	[OP_DMP] = "DMP",
	[OP_PRT] = "PRT",
	[OP_FPR] = "FPR",

	[OP_HLT] = "HLT",
};

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
