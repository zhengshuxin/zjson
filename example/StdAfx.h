#ifndef __STD_AFX_INCLUDE_H__
#define __STD_AFX_INCLUDE_H__

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)

#include <windows.h>

# define snprintf _snprintf
# define vsnprintf _vsnprintf

#else

#include <getopt.h>
#include <sys/time.h>
#include <sys/types.h>

#endif
#endif /* __STD_AFX_INCLUDE_H__ */
