#include "main.h"

static const char *platform(void) {
#if defined(PLATFORM_WINDOWS)
	return "Windows";
#elif defined(PLATFORM_APPLE)
	return "Apple";
#elif defined(PLATFORM_LINUX)
	return "Linux";
#elif defined(PLATFORM_UNIX)
	return "Unix";
#else
	return "Unknown";
#endif
}

static const char *compiler(void) {
#if defined(COMPILER_GCC)
	return "gcc";
#elif defined(COMPILER_CLANG)
	return "clang";
#elif defined(COMPILER_MSVC)
	return "MSVC";
#else
	return "Unknown";
#endif
}

void usage(void) {
	printf(ASCII_LOGO" v%i.%i.%i\n\n"
	       "Github: "GITHUB_LINK"\n"
	       "Usage: "APP_NAME" [FILE] [OPTIONS]\n"
	       "Options:\n"
	       "  -h, --help     Show this message\n"
	       "  -v, --version  Print the version\n"
	       "  --noW          Dont show warnings\n"
	       "  -d, --debug    Enable debug mode\n",
	       VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

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
	PARSE_FMT_INTO(p_fmt, msg, 256);

	fprintf(stderr, "Error: %s\n", msg);
}

void try(const char *p_flag) {
	fprintf(stderr, "Try '"APP_NAME" %s'\n", p_flag);
}

int main(int p_argc, char **p_argv) {
	const char *path     = NULL;
	bool        warnings = true;
	bool        debug    = false;

	for (int i = 1; i < p_argc; ++ i) {
		if (strcmp(p_argv[i], "-h") == 0 || strcmp(p_argv[i], "--help") == 0)
			usage();
		else if (strcmp(p_argv[i], "-v") == 0 || strcmp(p_argv[i], "--version") == 0)
			version();
		else if (strcmp(p_argv[i], "-d") == 0 || strcmp(p_argv[i], "--debug") == 0)
			debug = true;
		else if (strcmp(p_argv[i], "--noW") == 0)
			warnings = false;
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
	vm_init(&vm);
	vm_exec_from_file(&vm, path, warnings, debug);
	vm_destroy(&vm);

	return vm.ex;
}
