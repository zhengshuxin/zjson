#ifndef __STD_AFX_INCLUDE_H__
#define __STD_AFX_INCLUDE_H__

#include <float.h>			/* DBL_MAX_10_EXP */
#include <ctype.h>
#include <limits.h>			/* CHAR_BIT */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

# define snprintf _snprintf
# define vsnprintf _vsnprintf

int strcasecmp(const char *s1, const char *s2);

#else

#include <unistd.h>

#endif
#endif /* __STD_AFX_INCLUDE_H__ */
