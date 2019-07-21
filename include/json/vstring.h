#ifndef VSTRING_INCLUDE_H
#define VSTRING_INCLUDE_H

#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DBUF DBUF;
typedef struct VSTRING VSTRING;

struct VSTRING {
	char    *buf;
	unsigned len;
	unsigned cnt;
};

#define VSTRING_STR(vp) ((vp) ? ((vp)->buf) : "")
#define VSTRING_LEN(vp) ((vp) ? ((vp)->cnt) : 0)

#define VSTRING_RESET(vp) do { if ((vp)) { ((vp)->cnt = 0); } } while (0)

VSTRING *vstring_alloc(DBUF *dbuf, size_t len);
VSTRING *vstring_strcpy(DBUF *dbuf, VSTRING *to, const char *s);
VSTRING *vstring_strcat(DBUF *dbuf, VSTRING *to, const char *s);
VSTRING *vstring_memcat(DBUF *dbuf, VSTRING *to, const char *s, size_t len);

#define VSTRING_EXTEND(dp, vp, inc) do { \
	char *_buf_;  \
	(vp)->len += inc > 16 ? inc : 16;  \
	_buf_ = (char*) dbuf_alloc((dp), (vp)->len);  \
	memcpy(_buf_,  (vp)->buf, (vp)->cnt);  \
	(vp)->buf = _buf_;  \
} while (0)

#define VSTRING_ADDCH(dp, vp, ch) do {  \
	if ((vp) == NULL) {  \
		(vp) = vstring_alloc((dp), 4);  \
	}  \
	if ((vp)->cnt + 1 > (vp)->len) {  \
		VSTRING_EXTEND(dp, vp, (vp)->len);  \
	}  \
	(vp)->buf[(vp)->cnt++] = (char) ch;  \
} while (0)

#define VSTRING_TERMINATE(dp, vp) do {  \
	if ((vp)->cnt + 1 > (vp)->len) {  \
		VSTRING_EXTEND(dp, vp, (vp)->cnt + 1 - (vp)->len);  \
	}  \
	(vp)->buf[(vp)->cnt] = 0;  \
} while (0)

#ifdef __cplusplus
}
#endif

#endif
