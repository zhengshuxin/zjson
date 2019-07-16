#ifndef	DBUF_INCLUDE_H
#define	DBUF_INCLUDE_H

#include <stdlib.h>

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct DBUF DBUF;

/**
 * 创建内存池对象
 * @param block_size {size_t} 内存池中每个连续内存块的大小（字节）
 * @return {DBUF*} 返回非 NULL 对象
 */
DBUF *dbuf_create(size_t block_size);

/**
 * 重置内存池状态，会将多余的内存数据块释放
 * @param dbuf {DBUF*} 内存池对象
 * @param off {size_t} 要求保留的最小内存相对偏移位置
 * @return {int} 返回 0 表示操作成功，非 0 表示失败
 */
int  dbuf_reset(DBUF *dbuf, size_t off);

/**
 * 销毁内存池
 * @param dbuf {DBUF*} 对象池对象
 */
void dbuf_destroy(DBUF *dbuf);

/**
 * 分配指定长度的内存
 * @param dbuf {DBUF*} 对象池对象
 * @param  len {size_t} 需要分配的内存大小
 * @return {void*} 新分配的内存地址
 */
void *dbuf_alloc(DBUF *dbuf, size_t len);

/**
 * 分配指定长度的内存并将内存区域清零
 * @param dbuf {DBUF*} 对象池对象
 * @param len {size_t} 需要分配的内存长度
 * @return {void*} 新分配的内存地址
 */
void *dbuf_calloc(DBUF *dbuf, size_t len);

/**
 * 根据输入的字符串动态创建新的内存并将字符串进行复制，类似于 strdup
 * @param dbuf {DBUF*} 对象池对象
 * @param s {const char*} 源字符串
 * @return {char*} 新复制的字符串地址
 */
char *dbuf_strdup(DBUF *dbuf, const char *s);

/**
 * 根据输入的字符串动态创建新的内存并将字符串进行复制，类似于 strdup
 * @param dbuf {DBUF*} 对象池对象
 * @param s {const char*} 源字符串
 * @param len {size_t} 限定的最大字符串长度
 * @return {char*} 新复制的字符串地址
 */
char *dbuf_strndup(DBUF *dbuf, const char *s, size_t len);

/**
 * 根据输入的内存数据动态创建内存并将数据进行复制
 * @param dbuf {DBUF*} 对象池对象
 * @param addr {const void*} 源数据内存地址
 * @param len {size_t} 源数据长度
 * @return {void*} 新复制的数据地址
 */
void *dbuf_memdup(DBUF *dbuf, const void *addr, size_t len);

/**
 * 归还由内存池分配的内存
 * @param dbuf {DBUF*} 对象池对象
 * @param addr {const void*} 由内存池分配的内存地址
 * @return {int} 如果该内存地址非内存池分配或释放多次，则返回 -1，操作成功则
 *  返回 0
 */
int dbuf_free(DBUF *dbuf, const void *addr);

/**
 * 保留由内存池分配的某段地址，以免当调用 reset 时被提前释放掉
 * @param dbuf {DBUF*} 对象池对象
 * @param addr {const void*} 由内存池分配的内存地址
 * @return {int} 操作成功则返回 0，如果该内存地址非内存池分配，则返回 -1
 */
int dbuf_keep(DBUF *dbuf, const void *addr);

/**
 * 取消保留由内存池分配的某段地址，以便于调用 dbuf_reset 时被释放掉
 * @param dbuf {DBUF*} 对象池对象
 * @param addr {const void*} 由内存池分配的内存地址
 * @return {int} 操作成功则返回 0，如果该内存地址非内存池分配，则返回 -1
 */
int dbuf_unkeep(DBUF *dbuf, const void *addr);

/**
 * 内部测试用函数
 */
void dbuf_test(size_t max);

#ifdef	__cplusplus
}
#endif

#endif
