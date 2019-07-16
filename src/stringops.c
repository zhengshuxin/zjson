#include <ctype.h>
#include <stdlib.h>
#include "stringops.h"

int is_alldig(const char *s)
{
	if (s == NULL || *s == 0) {
		return 0;
	}
	for (; *s != 0; s++) {
		if (!isdigit(*s)) {
			return 0;
		}
	}
	return 1;
}

int is_double(const char *s)
{
	if (s == NULL || *s == 0) {
		return 0;
	}
	if (*s == '-' || *s == '+') {
		s++;
	}
	if (*s == 0 || *s == '.') {
		return 0;
	}

	while (*s != 0) {
		if (*s == '.') {
			s++;
			if (*s == 0) {
				return 0;
			}
			break;
		}
		if (!isdigit(*s)) {
			return 0;
		}
		s++;
	}

	if (*s == 0) {
		return 1;
	}

	while (*s != 0) {
		if (!isdigit(*s)) {
			return 0;
		}
		s++;
	}

	return 1;
}
