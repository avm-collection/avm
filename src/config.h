#ifndef CONFIG_H__HEADER_GUARD__
#define CONFIG_H__HEADER_GUARD__

#include "platform.h"

#ifdef PLATFORM_LINUX
#	define USES_READLINE
#endif

#define APP_NAME    "avm"
#define TITLE       "AVM"
#define GITHUB_LINK "https://github.com/avm-collection/avm"

#define VERSION_MAJOR 1
#define VERSION_MINOR 10
#define VERSION_PATCH 6

#define ASCII_LOGO \
	" __________________ \n" \
	"|  ______________  |\n" \
	"| |     __ _     | |\n" \
	"| |    / _` |    | |\n" \
	"| |   | (_| |    | |\n" \
	"| |    \\__,_|    | |\n" \
	"| |______________| |\n" \
	"|__________________|\n" \
	"       ______       \n" \
	"      |______|      "

#endif
