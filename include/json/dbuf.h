#ifndef	DBUF_INCLUDE_H
#define	DBUF_INCLUDE_H

#include <stdlib.h>

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct DBUF DBUF;

/**
 * �����ڴ�ض���
 * @param block_size {size_t} �ڴ����ÿ�������ڴ��Ĵ�С���ֽڣ�
 * @return {DBUF*} ���ط� NULL ����
 */
DBUF *dbuf_create(size_t block_size);

/**
 * �����ڴ��״̬���Ὣ������ڴ����ݿ��ͷ�
 * @param dbuf {DBUF*} �ڴ�ض���
 * @param off {size_t} Ҫ��������С�ڴ����ƫ��λ��
 * @return {int} ���� 0 ��ʾ�����ɹ����� 0 ��ʾʧ��
 */
int  dbuf_reset(DBUF *dbuf, size_t off);

/**
 * �����ڴ��
 * @param dbuf {DBUF*} ����ض���
 */
void dbuf_destroy(DBUF *dbuf);

/**
 * ����ָ�����ȵ��ڴ�
 * @param dbuf {DBUF*} ����ض���
 * @param  len {size_t} ��Ҫ������ڴ��С
 * @return {void*} �·�����ڴ��ַ
 */
void *dbuf_alloc(DBUF *dbuf, size_t len);

/**
 * ����ָ�����ȵ��ڴ沢���ڴ���������
 * @param dbuf {DBUF*} ����ض���
 * @param len {size_t} ��Ҫ������ڴ泤��
 * @return {void*} �·�����ڴ��ַ
 */
void *dbuf_calloc(DBUF *dbuf, size_t len);

/**
 * ����������ַ�����̬�����µ��ڴ沢���ַ������и��ƣ������� strdup
 * @param dbuf {DBUF*} ����ض���
 * @param s {const char*} Դ�ַ���
 * @return {char*} �¸��Ƶ��ַ�����ַ
 */
char *dbuf_strdup(DBUF *dbuf, const char *s);

/**
 * ����������ַ�����̬�����µ��ڴ沢���ַ������и��ƣ������� strdup
 * @param dbuf {DBUF*} ����ض���
 * @param s {const char*} Դ�ַ���
 * @param len {size_t} �޶�������ַ�������
 * @return {char*} �¸��Ƶ��ַ�����ַ
 */
char *dbuf_strndup(DBUF *dbuf, const char *s, size_t len);

/**
 * ����������ڴ����ݶ�̬�����ڴ沢�����ݽ��и���
 * @param dbuf {DBUF*} ����ض���
 * @param addr {const void*} Դ�����ڴ��ַ
 * @param len {size_t} Դ���ݳ���
 * @return {void*} �¸��Ƶ����ݵ�ַ
 */
void *dbuf_memdup(DBUF *dbuf, const void *addr, size_t len);

/**
 * �黹���ڴ�ط�����ڴ�
 * @param dbuf {DBUF*} ����ض���
 * @param addr {const void*} ���ڴ�ط�����ڴ��ַ
 * @return {int} ������ڴ��ַ���ڴ�ط�����ͷŶ�Σ��򷵻� -1�������ɹ���
 *  ���� 0
 */
int dbuf_free(DBUF *dbuf, const void *addr);

/**
 * �������ڴ�ط����ĳ�ε�ַ�����⵱���� reset ʱ����ǰ�ͷŵ�
 * @param dbuf {DBUF*} ����ض���
 * @param addr {const void*} ���ڴ�ط�����ڴ��ַ
 * @return {int} �����ɹ��򷵻� 0��������ڴ��ַ���ڴ�ط��䣬�򷵻� -1
 */
int dbuf_keep(DBUF *dbuf, const void *addr);

/**
 * ȡ���������ڴ�ط����ĳ�ε�ַ���Ա��ڵ��� dbuf_reset ʱ���ͷŵ�
 * @param dbuf {DBUF*} ����ض���
 * @param addr {const void*} ���ڴ�ط�����ڴ��ַ
 * @return {int} �����ɹ��򷵻� 0��������ڴ��ַ���ڴ�ط��䣬�򷵻� -1
 */
int dbuf_unkeep(DBUF *dbuf, const void *addr);

/**
 * �ڲ������ú���
 */
void dbuf_test(size_t max);

#ifdef	__cplusplus
}
#endif

#endif
