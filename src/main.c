#include "main.h"

static const char *platform(void) {
#if defined(__PLATFORM_WINDOWS__)
	return "Windows";
#elif defined(__PLATFORM_APPLE__)
	return "Apple";
#elif defined(__PLATFORM_LINUX__)
	return "Linux";
#elif defined(__PLATFORM_UNIX__)
	return "Unix";
#else
	return "Unknown";
#endif
}

static const char *compiler(void) {
#if defined(__COMPILER_GCC__)
	return "gcc";
#elif defined(__COMPILER_CLANG__)
	return "clang";
#elif defined(__COMPILER_MSVC__)
	return "MSVC";
#else
	return "Unknown";
#endif
}

void usage(void) {
	puts("Usage: "APP_NAME" [FILE] [OPTIONS]\n"
	     "Options:\n"
	     "  -h, --help     Show this message\n"
	     "  -v, --version  Print the version");

	exit(EXIT_SUCCESS);
}

void version(void) {
	printf(APP_NAME" %i.%i.%i\n"
	       "  Compiled on "__TIME__" "__DATE__"\n"
	       "  With %s\n"
	       "  For platform %s\n",
	       VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
	       compiler(),
	       platform());

	exit(EXIT_SUCCESS);
}

void error(const char *p_fmt, ...) {
	char    msg[1024];
	va_list args;

	va_start(args, p_fmt);
	vsnprintf(msg, sizeof(msg), p_fmt, args);
	va_end(args);

	fprintf(stderr, "Error: %s\n", msg);
}

void try(const char *p_flag) {
	fprintf(stderr, "Try '"APP_NAME" %s'\n", p_flag);
}

int main(int p_argc, char **p_argv) {
	const char *path = NULL;

	for (int i = 1; i < p_argc; ++ i) {
		if (strcmp(p_argv[i], "-h") == 0 || strcmp(p_argv[i], "--help") == 0)
			usage();
		else if (strcmp(p_argv[i], "-v") == 0 || strcmp(p_argv[i], "--version") == 0)
			version();
		else if (path != NULL) {
			error("Unexpected argument '%s'", p_argv[i]);
			try("-h");

			exit(EXIT_FAILURE);
		} else
			path = p_argv[i];
	}

	if (path == NULL) {
		error("No input file specified");
		try("-h");

		exit(EXIT_FAILURE);
	}

	struct vm vm;
	vm_exec_from_file(&vm, path);

	return *vm.ex;
}
