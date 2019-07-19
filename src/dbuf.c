#include "StdAfx.h"
#include "json/dbuf.h"

typedef struct BLOCK {
        struct BLOCK *next;
	short  used;
	short  keep;
	size_t size;
        char  *addr;
        char   buf[1];
} BLOCK;

struct DBUF {
        size_t block_size;
	size_t off;
	size_t huge;
	size_t count;
        BLOCK *head;
	char   buf[1];
};

DBUF *dbuf_create(size_t block_size)
{
	DBUF  *dbuf;
	size_t size;
	int    page_size;

#ifdef UNIX
	page_size = getpagesize();
#elif defined(_WIN32) || defined(_WIN64)
	SYSTEM_INFO info;

	memset(&info, 0, sizeof(SYSTEM_INFO));
	GetSystemInfo(&info);
	page_size = info.dwPageSize;
	if (page_size <= 0) {
		page_size = 4096;
	}
#else
	page_size = 4096;
#endif

	size = (block_size / (size_t) page_size) * (size_t) page_size;
	if (size < (size_t) page_size) {
		size = page_size;
	}

	size -= sizeof(BLOCK);

#ifdef	USE_VALLOC
	dbuf = (DBUF*) valloc(sizeof(struct DBUF) + sizeof(BLOCK) + size);
#else
	dbuf = (DBUF*) malloc(sizeof(struct DBUF) + sizeof(BLOCK) + size);
#endif

	dbuf->block_size = size;
	dbuf->off        = 0;
	dbuf->huge       = 0;
	dbuf->head       = (BLOCK*) dbuf->buf;
	dbuf->head->next = NULL;
	dbuf->head->keep = 1;
	dbuf->head->used = 0;
	dbuf->head->size = size;
	dbuf->head->addr = dbuf->head->buf;
	dbuf->count      = 1;

	return dbuf;
}

void dbuf_destroy(DBUF *dbuf)
{
	BLOCK *iter = dbuf->head, *tmp;

	while (iter) {
		tmp = iter;
		iter = iter->next;
		if ((char*) tmp == dbuf->buf) {
			break;
		}
		if (tmp->size > dbuf->block_size) {
			dbuf->huge--;
		}
		free(tmp);
	}

	free(dbuf);
}

int dbuf_reset(DBUF *dbuf, size_t off)
{
	size_t n;
	BLOCK *iter = dbuf->head, *tmp;

	if (off > dbuf->off) {
		printf("warning: %s(%d) off(%ld) > dbuf->off(%ld)\r\n",
			__FUNCTION__, __LINE__, (long) off, (long) dbuf->off);
		return -1;
	} else if (off == dbuf->off) {
		return 0;
	}

	while (1) {
		/* 如果当前内存块有保留内存区，则保留整个内存块 */
		if (iter->keep) {
			break;
		}

		/* 计算当前内存块被使用的内存大小 */
		n = iter->addr - iter->buf;

		/* 当 off 相对偏移量在当前内存块时，则退出循环 */
		if (dbuf->off <= off + n) {
			iter->addr -= dbuf->off - off;
			dbuf->off  = off;
			break;
		}

		/* 除头部节点外，至少再保留一个空闲的节点，以便重复使用 */
		if (dbuf->count == 2) {
			BLOCK *first = (BLOCK*) dbuf->buf;
			dbuf->head->addr = dbuf->head->buf;
			dbuf->head->used = 0;
			dbuf->off        = first->size;
			break;
		}

		/* 保留当前内存块指针以便于下面进行释放 */
		tmp = iter;
		/* 指向下一个内存块地址 */
		iter = iter->next;

		dbuf->head = iter;

		/* off 为下一个内存块的 addr 所在的相对偏移位置  */
		dbuf->off -=n;

		if (tmp->size > dbuf->block_size) {
			dbuf->huge--;
		}

		free(tmp);
		dbuf->count--;
	}

	return 0;
}

int dbuf_free(DBUF *dbuf, const void *addr)
{
	const char *ptr = (const char*) addr;
	BLOCK *iter = dbuf->head, *prev = iter;

	while (iter) {
		if (ptr < iter->addr && ptr >= iter->buf) {
			iter->used--;
			break;
		}

		prev = iter;
		iter = iter->next;
	}

	if (iter == NULL) {
		printf("warning: %s(%d), not found addr: %p\r\n",
			__FUNCTION__, __LINE__, addr);
		return -1;
	}

	if (iter->used < 0) {
		printf("warning: %s(%d), used(%d) < 0\r\n",
			__FUNCTION__, __LINE__, iter->used);
		return -1;
	}

	if (iter->used > 0 || iter->keep) {
		return 0;
	}

	/* should free the BLOCK */

	if (iter == dbuf->head) {
		dbuf->head = iter->next;
	} else {
		prev->next = iter->next;
	}

	dbuf->off -= iter->addr - iter->buf;

	if (iter->size > dbuf->block_size) {
		dbuf->huge--;
	}

	free(iter);
	return 1;
}

static BLOCK *block_alloc(DBUF *dbuf, size_t length)
{
#ifdef	USE_VALLOC
	BLOCK *block = (BLOCK*) valloc(sizeof(BLOCK) + length);
#else
	BLOCK *block = (BLOCK*) malloc(sizeof(BLOCK) + length);
#endif

	block->next = dbuf->head;
	block->used = 0;
	block->keep = 0;
	block->size = length;
	block->addr = block->buf;

	dbuf->head  = block;
	if (length > dbuf->block_size) {
		dbuf->huge++;
	}

	dbuf->count++;
	return block;
}

void *dbuf_alloc(DBUF *dbuf, size_t length)
{
	void  *ptr;
	BLOCK *block;

	length += 4 - length % 4;

	if (length > dbuf->block_size) {
		block = block_alloc(dbuf, length);
	} else if (dbuf->head == NULL) {
		block = block_alloc(dbuf, dbuf->block_size);
	} else if (dbuf->block_size < ((char*) dbuf->head->addr
		- (char*) dbuf->head->buf) + length) {

		block = block_alloc(dbuf, dbuf->block_size);
	} else {
		block = dbuf->head;
	}

	ptr = block->addr;
	block->addr = (char*) block->addr + length;
	dbuf->off += length;
	block->used++;

	return ptr;
}

void *dbuf_calloc(DBUF *dbuf, size_t length)
{
	void *ptr;

	ptr = dbuf_alloc(dbuf, length);
	if (ptr) {
		memset(ptr, 0, length);
	}
	return ptr;
}

char *dbuf_strdup(DBUF *dbuf, const char *s)
{
	size_t len = strlen(s);
	char  *ptr = (char*) dbuf_alloc(dbuf, len + 1);

	memcpy(ptr, s, len);
	ptr[len] = 0;
	return ptr;
}

char *dbuf_strndup(DBUF *dbuf, const char *s, size_t len)
{
	char *ptr;
	size_t n = strlen(s);

	if (n > len) {
		n = len;
	}
	ptr = (char*) dbuf_alloc(dbuf, n + 1);
	memcpy(ptr, s, n);
	ptr[n] = 0;
	return ptr;
}

void *dbuf_memdup(DBUF *dbuf, const void *addr, size_t len)
{
	void *ptr = dbuf_alloc(dbuf, len);

	memcpy(ptr, addr, len);
	return ptr;
}

int dbuf_keep(DBUF *dbuf, const void *addr)
{
	const char *ptr = (const char*) addr;
	BLOCK *iter = dbuf->head;

	while (iter) {
		if (ptr < iter->addr && ptr >= iter->buf) {
			iter->keep++;
			if (iter->keep <= iter->used) {
				return 0;
			}

			printf("warning: %s(%d), keep(%d) > used(%d)\r\n",
				__FUNCTION__, __LINE__, iter->keep, iter->used);
			return -1;
		}

		iter = iter->next;
	}

	printf("warning: %s(%d), not found addr: %p\r\n",
		__FUNCTION__, __LINE__, addr);
	return -1;
}

int dbuf_unkeep(DBUF *dbuf, const void *addr)
{
	const char *ptr = (const char*) addr;
	BLOCK *iter = dbuf->head;

	while (iter) {
		if (ptr < iter->addr && ptr >= iter->buf) {
			iter->keep--;
			if (iter->keep >= 0) {
				return 0;
			}

			printf("warning: %s(%d), keep(%d) < 0\r\n",
				__FUNCTION__, __LINE__, iter->keep);
			return -1;
		}

		iter = iter->next;
	}

	printf("warning: %s(%d), not found addr: %p\r\n",
		__FUNCTION__, __LINE__, addr);
	return -1;
}

void dbuf_test(size_t max)
{
	DBUF *dbuf;
	size_t i, n = 1000000, len, j, k;

	for (j = 0; j < max; j++) {
		printf("begin alloc, max: %d\n", (int) n);
		dbuf = dbuf_create(0);
		for (i = 0; i < n; i++) {
			k = i % 10;
			switch (k) {
			case 0:
				len = 1024;
				break;
			case 1:
				len = 1999;
				break;
			case 2:
				len = 999;
				break;
			case 3:
				len = 230;
				break;
			case 4:
				len = 199;
				break;
			case 5:
				len = 99;
				break;
			case 6:
				len = 19;
				break;
			case 7:
				len = 29;
				break;
			case 8:
				len = 9;
				break;
			case 9:
				len = 399;
				break;
			default:
				len = 88;
				break;
			}
			(void) dbuf_alloc(dbuf, len);
		}
		printf("alloc over now, sleep(10)\n");
		//sleep(10);
		dbuf_destroy(dbuf);
	}
}
