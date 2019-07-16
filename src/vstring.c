#include <string.h>

#include "json/dbuf.h"
#include "json/vstring.h"

VSTRING *vstring_alloc(DBUF *dbuf, size_t len)
{
	VSTRING *s = (VSTRING *) dbuf_alloc(dbuf, sizeof(VSTRING));

	s->buf = (char*) dbuf_alloc(dbuf, len);
	s->len = (unsigned) len;
	s->cnt = 0;
	return s;
}

VSTRING *vstring_strcpy(DBUF *dbuf, VSTRING *to, const char *s)
{
	size_t len;

	if (s == NULL || *s == 0) {
		return to;
	}

	len = strlen(s);
	if (to == NULL) {
		to = vstring_alloc(dbuf, len + 1);
	} else if (len + 1 > to->len) {
		VSTRING_EXTEND(dbuf, to, len + 1 - to->len);
	}

	memcpy((char*) to->buf, s, len);
	to->cnt = (unsigned) len;
	to->buf[to->cnt] = 0;
	return to;
}

VSTRING *vstring_strcat(DBUF *dbuf, VSTRING *to, const char *s)
{
	return vstring_memcat(dbuf, to, s, strlen(s));
}

VSTRING *vstring_memcat(DBUF *dbuf, VSTRING *to, const char *s, size_t len)
{
	if (s == NULL || len == 0) {
		return to;
	}

	if (to == NULL) {
		to = vstring_alloc(dbuf, len + 1);
	} else if (to->cnt + len + 1 > to->len) {
		VSTRING_EXTEND(dbuf, to, to->cnt + len + 1 - to->len);
	}

	memcpy((char*) to->buf + to->cnt, s, len);
	to->cnt += len;
	to->buf[to->cnt] = 0;
	return to;
}
