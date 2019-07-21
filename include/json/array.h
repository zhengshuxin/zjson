#ifndef	ARRAY_INCLUDE_H
#define	ARRAY_INCLUDE_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "json_define.h"
#include "iterator.h"

/**
 * ��̬�������Ͷ���
 */
typedef	struct ARRAY ARRAY;

struct ARRAY{
	int     capacity;	/**< items ����ռ��С */
	int     count;		/**< items �к���Ԫ�صĸ��� */
	void  **items;		/**< ��̬���� */

	/* for iterator */

	/* ȡ������ͷ���� */
	void *(*iter_head)(ITER*, struct ARRAY*);
	/* ȡ��������һ������ */
	void *(*iter_next)(ITER*, struct ARRAY*);
	/* ȡ������β���� */
	void *(*iter_tail)(ITER*, struct ARRAY*);
	/* ȡ��������һ������ */
	void *(*iter_prev)(ITER*, struct ARRAY*);
};

/**
 * ����һ����̬����
 * @param init_size {int} ��̬����ĳ�ʼ��С
 * @return {ARRAY*} ��̬����ָ��
 */
JSON_API ARRAY *array_create(int init_size);

/**
 * �ͷŵ���̬�����ڵĳ�Ա�������������ͷŶ�̬�������
 * @param a {ARRAY*} ��̬����ָ��
 * @param free_fn {void (*)(void*)} �����ͷŶ�̬�����ڳ�Ա�������ͷź���ָ��
 */
JSON_API void array_clean(ARRAY *a, void (*free_fn)(void *));

/**
 * �ͷŵ���̬�����ڵĳ�Ա���������ͷŶ�̬������󣬵�������󴴽� dbuf ����
 * ʱ��������������ͷŽ������ͷ� dbuf ʱ���ͷ�
 * @param a {ARRAY*} ��̬����ָ��
 * @param free_fn {void (*)(void*)} �����ͷŶ�̬�����ڳ�Ա�������ͷź���ָ��
 */
JSON_API void array_free(ARRAY *a, void (*free_fn)(void *));

/**
 * ��̬����β����Ӷ�̬��Ա����
 * @param a {ARRAY*} ��̬����ָ��
 * @param obj {void*} ��̬��Ա����
 * @return {int} >=0: �ɹ�, ����ֵΪ��Ԫ���������е��±�λ�ã�-1: ʧ��
 */
JSON_API int array_append(ARRAY *a, void *obj);

/**
 * �Ӷ�̬�����е�ĳ���±�λ��ȡ����̬����
 * @param a {ARRAY*} ��̬����ָ��
 * @param idx {int} �±�λ�ã�����Խ�磬���򷵻�-1
 * @return {void*} != NULL: �ɹ���== NULL: �����ڻ�ʧ��
 */
JSON_API void *array_index(const ARRAY *a, int idx);

/**
 * ��õ�ǰ��̬�����ж�̬����ĸ���
 * @param a {ARRAY*} ��̬����ָ��
 * @return {int} ��̬�����ж�̬����ĸ���
 */
JSON_API int array_size(const ARRAY *a);

#ifdef  __cplusplus
}
#endif

#endif
