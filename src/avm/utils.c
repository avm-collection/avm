#include "utils.h"

char *strtrim(char *p_str) {
	/* Trim leading whitespaces */
	while (isspace(*p_str))
		++ p_str;

	/* Trim trailing whitespaces */
	char *end = p_str + strlen(p_str) - 1;
	while (end > p_str && isspace(*end))
		-- end;

	end[1] = '\0';

	return p_str;
}
