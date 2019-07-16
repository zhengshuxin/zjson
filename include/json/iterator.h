#ifndef	ITERATOR_INCLUDE_H
#define	ITERATOR_INCLUDE_H

typedef struct ITER ITER;

/**
 * ACL �������ݽṹ�õ�ͨ�õ������ṹ����
 */
struct ITER {
	void *ptr;		/**< ������ָ��, ��������� */
	void *data;		/**< �û�����ָ�� */
	int   dlen;		/**< �û����ݳ���, ʵ���߿����ô�ֵҲ�ɲ����� */
	const char *key;	/**< ��Ϊ��ϣ��ĵ�����, ��Ϊ��ϣ��ֵ��ַ */
	int   klen;		/**< ��ΪBINHASH������, ��Ϊ������ */
	int   i;		/**< ��ǰ�������������е�λ������ */
	int   size;		/**< ��ǰ������Ԫ���ܸ��� */
};

/**
 * �������������Ԫ��
 * @param iter {ITER}
 * @param container {void*} ������ַ
 * @examples: samples/iterator/
 */
#define	FOREACH(iter, container)  \
        for ((container)->iter_head(&(iter), (container));  \
             (iter).ptr;  \
             (container)->iter_next(&(iter), (container)))

/**
 * �������������Ԫ��
 * @param iter {ITER}
 * @param container {void*} ������ַ
 * @examples: samples/iterator/
 */
#define	FOREACH_REVERSE(iter, container)  \
        for ((container)->iter_tail(&(iter), (container));  \
             (iter).ptr;  \
             (container)->iter_prev(&(iter), (container)))

/**
 * ��õ�ǰ����ָ����ĳ���������ĳ�Ա�ṹ���Ͷ���
 * @param iter {ITER}
 * @param container {void*} ������ַ
 */
#define	ITER_INFO(iter, container)  \
	(container)->iter_info(&(iter), (container))

#define	foreach_reverse	FOREACH_REVERSE
#define	foreach		FOREACH
#define	iter_info	ITER_INFO

#endif
