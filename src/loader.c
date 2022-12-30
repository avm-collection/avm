#include "loader.h"

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

void vm_exec_from_file(struct vm *p_vm, const char *p_path, bool p_warnings, bool p_debug) {
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

	if (meta.ver[0] != VERSION_MAJOR && p_warnings)
		VM_WARN(stderr, "'%s' major version is %i, your avm major version is %i",
		        p_path, meta.ver[0], VERSION_MAJOR);
	else if (meta.ver[1] > VERSION_MINOR && p_warnings)
		VM_WARN(stderr, "'%s' minor version is %i, greater than your avm minor version which is %i",
		        p_path, meta.ver[1], VERSION_MINOR);

	// Ignore the patch version

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

	vm_load_from_mem(p_vm, program, program_size, entry_point);
	if (p_debug)
		vm_debug(p_vm);
	else
		vm_run(p_vm);

	free(program);
}
