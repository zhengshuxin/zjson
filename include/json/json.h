#ifndef JSON_INCLUDE_H
#define JSON_INCLUDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "json_define.h"
#include "ring.h"
#include "iterator.h"

typedef struct DBUF DBUF;
typedef struct VSTRING VSTRING;
typedef	struct ARRAY ARRAY;
typedef struct JSON JSON;
typedef struct JSON_NODE JSON_NODE;

struct JSON_NODE {
	VSTRING   *ltag;		/**< ��ǩ�� */
	VSTRING   *text;		/**< ���ڵ�ΪҶ�ڵ�ʱ���ı����ݷǿ� */
	JSON_NODE *tag_node;		/**< ����ǩֵΪ json �ڵ�ʱ����ǿ� */
	JSON_NODE *parent;		/**< ���ڵ� */
	RING       children;		/**< �ӽڵ㼯�� */
	RING       node;		/**< ��ǰ�ڵ� */
	unsigned int   depth:30;	/**< ��ǰ�ڵ����� */
	unsigned int   quoted:1;	/**< �ýڵ��Ƿ����Ű��� */
	unsigned short type;		/**< �ڵ����� */
#define	JSON_T_A_STRING      (1 << 0)
#define	JSON_T_A_NUMBER      (1 << 1)
#define	JSON_T_A_BOOL        (1 << 2)
#define	JSON_T_A_NULL        (1 << 3)
#define	JSON_T_A_DOUBLE      (1 << 4)

#define	JSON_T_STRING        (1 << 5)
#define	JSON_T_NUMBER        (1 << 6)
#define	JSON_T_BOOL          (1 << 7)
#define	JSON_T_NULL          (1 << 8)
#define JSON_T_DOUBLE        (1 << 9)

#define JSON_T_ARRAY         (1 << 10)
#define JSON_T_OBJ           (1 << 11)
#define JSON_T_LEAF          (1 << 12)
#define JSON_T_MEMBER        (1 << 13)
#define JSON_T_PAIR          (1 << 14)
#define	JSON_T_ELEMENT       (1 << 15)

	unsigned char left_ch;	/**< ���ڵ�ĵ�һ���ַ�: { or [ */
	unsigned char right_ch;	/**< ���ڵ�����һ���ַ�: } or ] */
};

enum {
	JSON_S_ROOT,		/**< ���ڵ� */
	JSON_S_OBJ,		/**< ��ǩ����ֵ */
	JSON_S_MEMBER,
	JSON_S_ARRAY,		/**< json �ڵ� array */
	JSON_S_ELEMENT,
	JSON_S_PAIR,		/**< name:value pair */
	JSON_S_NEXT,		/**< ��һ���ڵ� */
	JSON_S_TAG,		/**< �����ǩ�� */
	JSON_S_VALUE,		/**< �ڵ�ֵ������� */
	JSON_S_COLON,		/**< ð�� : */
	JSON_S_STRING,
	JSON_S_STREND
};

struct JSON {
	unsigned depth;		/**< ��ǰ Json ����������� */
	int   node_cnt;		/**< �ڵ�����, ���� root �ڵ� */
	JSON_NODE *root;	/**< json ���ڵ� */
	int   finish;		/**< �Ƿ�������� */
	unsigned flag;		/**< ��־λ */
#define	JSON_FLAG_PART_WORD	(1 << 0)	/**< �Ƿ���ݰ������ */
#define JSON_FLAG_ADD_SPACE	(1 << 1)	/**< ���� json ʱ�Ƿ���ո� */

	/* public: for iterator, ͨ�� foreach �����г������ӽڵ� */

	/* ȡ������ͷ���� */
	JSON_NODE *(*iter_head)(ITER*, JSON*);
	/* ȡ��������һ������ */
	JSON_NODE *(*iter_next)(ITER*, JSON*);
	/* ȡ������β���� */
	JSON_NODE *(*iter_tail)(ITER*, JSON*);
	/* ȡ��������һ������ */
	JSON_NODE *(*iter_prev)(ITER*, JSON*);

	/* private */

	int   status;		/**< ״̬����ǰ����״̬ */

	JSON_NODE *curr_node;	/**< ��ǰ���ڴ���� json �ڵ� */
	DBUF  *dbuf;		/**< �Ự�ڴ�ض��� */
	size_t keep;		/**< ���� dbuf_reset ʱϣ���������ڴ泤�� */

	VSTRING *buf;
	unsigned char quote;	/**< �� 0 ��ʾ ' �� " */
	unsigned backslash:1;	/**< ת���ַ� \ */
	unsigned part_word:1;	/**< ������ֵ���������־λ */
};

/*----------------------------- in json.c -----------------------------*/

/**
 * �� Json �ڵ������ֵתΪ�ַ�������
 * @param type {unsigned short}
 * @return {const char*}
 */
JSON_API const char *json_node_type(unsigned short type);
JSON_API int json_node_is_string(unsigned short type);
JSON_API int json_node_is_number(unsigned short type);
JSON_API int json_node_is_bool(unsigned short type);
JSON_API int json_node_is_null(unsigned short type);
JSON_API int json_node_is_obj(unsigned short type);
JSON_API int json_node_is_array(unsigned short type);

/**
 * ����һ�� json �ڵ�
 * @param json {JSON*} json ����
 * @return {JSON_NODE*} json �ڵ����
 */
JSON_API JSON_NODE *json_node_alloc(JSON *json);

/**
 * ��ĳ�� json �ڵ㼰���ӽڵ�� json ������ɾ��, ���ͷŸýڵ㼰���ӽڵ�
 * ��ռ�ռ亯�����ͷŸ� json �ڵ���ռ�ڴ�
 * @param json {JSON*} json ����
 * @param node {JSON_NODE*} json �ڵ�
 * @return {int} ����ɾ���Ľڵ����
 */
JSON_API int json_node_delete(JSON *json, JSON_NODE *node);

/**
 * ��ĳ�� json �ڵ�����ֵܽڵ�(���ֵܽڵ�����Ƕ����� json �ڵ�)
 * @param node1 {JSON_NODE*} �򱾽ڵ���� json �ڵ�
 * @param node2 {JSON_NODE*} ����ӵ��ֵ� json �ڵ�
 */
JSON_API void json_node_append(JSON_NODE *node1, JSON_NODE *node2);

/**
 * ��ĳ�� json �ڵ���Ϊ�ӽڵ����ĳ�� json �ڵ���
 * @param parent {JSON_NODE*} ���ڵ�
 * @param child {JSON_NODE*} �ӽڵ�
 */
JSON_API void json_node_add_child(JSON_NODE *parent, JSON_NODE *child);

/**
 * ��һ�� JSON ����� JSON �ڵ㸴���� JSON �����е�һ�� JSON �ڵ��У�������
 * Ŀ���´����� JSON �ڵ�
 * @param json {JSON*} Ŀ�� JSON ����
 * @param from {JSON_NODE*} Դ JSON �����һ�� JSON �ڵ�
 * @return {JSON_NODE*} ���طǿն���ָ��
 */
JSON_API JSON_NODE *json_node_duplicate(JSON *json, JSON_NODE *from);

/**
 * ���ĳ�� json �ڵ�ĸ��ڵ�
 * @param node {JSON_NODE*} json �ڵ�
 * @return {JSON_NODE*} ���ڵ�, ���Ϊ NULL ���ʾ�丸�ڵ㲻����
 */
JSON_API JSON_NODE *json_node_parent(JSON_NODE *node);

/**
 * ���ĳ�� json �ڵ�ĺ�һ���ֵܽڵ�
 * @param node {JSON_NODE*} json �ڵ�
 * @return {JSON_NODE*} ���� json �ڵ�ĺ�һ���ֵܽڵ�, ��ΪNULL���ʾ������
 */
JSON_API JSON_NODE *json_node_next(JSON_NODE *node);

/**
 * ���ĳ�� json �ڵ��ǰһ���ֵܽڵ�
 * @param node {JSON_NODE*} json �ڵ�
 * @return {JSON_NODE*} ���� json �ڵ��ǰһ���ֵܽڵ�, ��ΪNULL���ʾ������
 */
JSON_API JSON_NODE *json_node_prev(JSON_NODE *node);

/**
 * ����һ�� json ����
 * @return {JSON*} �´����� json ����
 */
JSON_API JSON *json_alloc(void);

/**
 * ����һ�� JSON �����һ�� JSON �ڵ㴴��һ���µ� JSON ����
 * @param node {JSON_NODE*} Դ JSON �����һ�� JSON �ڵ�
 * @return {JSON*} �´����� JSON ����
 */
JSON_API JSON *json_create(JSON_NODE *node);

/**
 * ��ĳһ�� JSON_NODE �ڵ���Ϊһ�� json ����ĸ��ڵ㣬
 * �Ӷ����Է���ر������ýڵ�ĸ����ӽڵ�(�ڱ��������е�����
 * �ڵ㲻�����ڵ�����)���ñ�����ʽ�б��ڵ���
 * ����ĳһ�� JSON_NODE �ڵ�ʱ���ܱ�����һ���ӽڵ������
 * @param json {JSON*} json ����
 * @param node {JSON_NODE*} JSON_NODE �ڵ�
 */
JSON_API void json_foreach_init(JSON *json, JSON_NODE *node);

/**
 * �ͷ�һ�� json ����, ͬʱ�ͷŸö��������ɵ����� json �ڵ�
 * @param json {JSON*} json ����
 */
JSON_API void json_free(JSON *json);

/**
 * ���� json ����������
 * @param json {JSON*} json ����
 */
JSON_API void json_reset(JSON *json);

/**
 * �� json �����л�õ�һ����������ǩ����ͬ�� json �ڵ�
 * @param json {JSON*} json ����
 * @param tag {const char*} ��ǩ����
 * @return {JSON_NODE*} ���������� json �ڵ�, ������ NULL ��
 *  ��ʾû�з��������� json �ڵ�
 */
JSON_API JSON_NODE *json_getFirstElementByTagName(JSON *json, const char *tag);

/**
 * �ͷ��� json_getElementsByTagName, json_getElementsByName,
 * �Ⱥ������صĶ�̬�������, ��Ϊ�ö�̬�����е�
 * Ԫ�ض��� JSON ������Ԫ�ص�����, �����ͷŵ��ö�̬�����, ֻҪ JSON
 * �����ͷ�, ��ԭ�����ڸ������е�Ԫ����Ȼ����ʹ��.
 * �������ͷ������ xml �ڵ�Ԫ��
 * @param a {ARRAY*} ��̬�������
 */
JSON_API void json_free_array(ARRAY *a);

/**
 * �� json �����л�����е���������ǩ����ͬ�� json �ڵ�ļ���
 * @param json {JSON*} json ����
 * @param tag {const char*} ��ǩ����
 * @return {ARRAY*} ���������� json �ڵ㼯��, ���� ��̬������, ������ NULL ��
 *  ��ʾû�з��������� json �ڵ�, �ǿ�ֵ��Ҫ���� json_free_array �ͷ�
 */
JSON_API ARRAY *json_getElementsByTagName(JSON *json, const char *tag);

/**
 * �� json �����л�����е�������༶��ǩ����ͬ�� json �ڵ�ļ���
 * @param json {JSON*} json ����
 * @param tags {const char*} �༶��ǩ������ '/' �ָ�������ǩ��������� json ���ݣ�
 *  { 'root': [
 *      'first': { 'second': { 'third': 'test1' } },
 *      'first': { 'second': { 'third': 'test2' } },
 *      'first': { 'second': { 'third': 'test3' } }
 *    ]
 *  }
 *  ����ͨ���༶��ǩ����root/first/second/third һ���Բ�����з��������Ľڵ�
 * @return {ARRAY*} ���������� json �ڵ㼯��, ���� ��̬������, ������ NULL ��
 *  ��ʾû�з��������� json �ڵ�, �ǿ�ֵ��Ҫ���� json_free_array �ͷ�
 */
JSON_API ARRAY *json_getElementsByTags(JSON *json, const char *tags);

/*------------------------- in json_parser.c ---------------------------*/

/**
 * ���� json ����, ���������Զ����� json �ڵ���
 * @param json {JSON*} json ����
 * @param data {const char*} �� '\0' ��β�������ַ���, ������������ json ����;
 *  Ҳ�����ǲ������� json ����, ����ѭ�����ô˺���, �����������ݳ���������; �ò���
 *  ��Ϊ NULL����ֱ�ӷ��ؿմ���ַ����˽�ֹΪ NULL
 * @return {const char*} �����������󣬸÷���ֵ��ʾʣ�����ݵ�ָ���ַ
 */
JSON_API const char* json_update(JSON *json, const char *data);

/**
 * �ж� JSON �����Ƿ����
 * @param json {JSON*} json ����
 * @return {int} ���ط� 0 ֵ��ʾ������ɣ������ʾδ���
 */
JSON_API int json_finish(JSON *json);

/*------------------------- in json_builder.c ----------------------------*/

/**
 * ���� json ����ʱ���� json Ҷ�ڵ�
 * @param json {JSON*} �� json_alloc / json_alloc1 ����
 * @param name {const char*} ��ǩ�����ǿ�
 * @param value {const char*} ��ǩֵ���ǿ�
 * @return {JSON_NODE*} �´����Ľڵ�������ͷ� JSON ����ʱ
 *  һ���ͷţ����Բ���Ҫ�����ͷ�
 */
JSON_API JSON_NODE *json_create_text(JSON *json, const char *name, const char *value);
#define json_create_leaf json_create_text

/**
 * ���� json ����ʱ���� json �������͵�Ҷ�ڵ�
 * @param json {JSON*} �� json_alloc / json_alloc1 ����
 * @param name {const char*} ��ǩ�����ǿ�
 * @param value {int} ��������ֵ
 * @return {JSON_NODE*} �´����Ľڵ�������ͷ� JSON ����ʱ
 *  һ���ͷţ����Բ���Ҫ�����ͷ�
 */
JSON_API JSON_NODE *json_create_bool(JSON *json, const char *name, int value);

/**
 * ���� json ����ʱ���� json null ���͵�Ҷ�ڵ�
 * @param json {JSON*} �� json_alloc / json_alloc1 ����
 * @param name {const char*} ��ǩ�����ǿ�
 * @return {JSON_NODE*} �´����Ľڵ�������ͷ� JSON ����ʱ
 *  һ���ͷţ����Բ���Ҫ�����ͷ�
 */
JSON_API JSON_NODE *json_create_null(JSON *json, const char *name);

/**
 * ���� json ����ʱ���� json int ���͵�Ҷ�ڵ�
 * @param json {JSON*} �� json_alloc / json_alloc1 ����
 * @param name {const char*} ��ǩ�����ǿ�
 * @param value {int64} �з�������ֵ
 * @return {JSON_NODE*} �´����Ľڵ�������ͷ� JSON ����ʱ
 *  һ���ͷţ����Բ���Ҫ�����ͷ�
 */
JSON_API JSON_NODE *json_create_int64(JSON *json, const char *name, long long value);

/**
 * ���� json ����ʱ���� json double ���͵�Ҷ�ڵ�
 * @param json {JSON*} �� json_alloc / json_alloc1 ����
 * @param name {const char*} ��ǩ�����ǿ�
 * @param value {double} �з�������ֵ
 * @return {JSON_NODE*} �´����Ľڵ�������ͷ� JSON ����ʱ
 *  һ���ͷţ����Բ���Ҫ�����ͷ�
 */
JSON_API JSON_NODE *json_create_double(JSON *json,
	const char *name, double value);

/**
 * ���� json ������ַ����ڵ㣬�� json �淶���ýڵ�ֻ�ܼ��������������
 * @param json {JSON*} �� json_alloc / json_alloc1 ����
 * @param text {const char*}
 * @return {JSON_NODE*} �´����Ľڵ�������ͷ� JSON ����ʱ
 *  һ���ͷţ����Բ���Ҫ�����ͷ�
 */
JSON_API JSON_NODE *json_create_array_text(JSON *json, const char *text);

/**
 * ���� json �������ֵ�ڵ㣬�� json �淶���ýڵ�ֻ�ܼ��������������
 * @param json {JSON*} �� json_alloc / json_alloc1 ����
 * @param text {int64}
 * @return {JSON_NODE*} �´����Ľڵ�������ͷ� JSON ����ʱ
 *  һ���ͷţ����Բ���Ҫ�����ͷ�
 */
JSON_API JSON_NODE *json_create_array_int64(JSON *json, long long value);
/**
 * ���� json �������ֵ�ڵ㣬�� json �淶���ýڵ�ֻ�ܼ��������������
 * @param json {JSON*} �� json_alloc / json_alloc1 ����
 * @param text {double}
 * @return {JSON_NODE*} �´����Ľڵ�������ͷ� JSON ����ʱ
 *  һ���ͷţ����Բ���Ҫ�����ͷ�
 */
JSON_API JSON_NODE *json_create_array_double(JSON *json, double value);

/**
 * ���� json ����Ĳ����ڵ㣬�� json �淶���ýڵ�ֻ�ܼ��������������
 * @param json {JSON*} �� json_alloc / json_alloc1 ����
 * @param value {int} �� 0 ��ʾ true�������ʾ false
 * @return {JSON_NODE*} �´����Ľڵ�������ͷ� JSON ����ʱ
 *  һ���ͷţ����Բ���Ҫ�����ͷ�
 */
JSON_API JSON_NODE *json_create_array_bool(JSON *json, int value);

/**
 * ���� json ����� null �ڵ㣬�� json �淶���ýڵ�ֻ�ܼ��������������
 * @param json {JSON*} �� json_alloc / json_alloc1 ����
 * @return {JSON_NODE*} �´����Ľڵ�������ͷ� JSON ����ʱ
 *  һ���ͷţ����Բ���Ҫ�����ͷ�
 */
JSON_API JSON_NODE *json_create_array_null(JSON *json);

/**
 * ���� json ����ʱ���� json ����(�������� {} �Ķ���)
 * @param json {JSON*} �� json_alloc / json_alloc1 ����
 * @return {JSON_NODE*} �´����Ľڵ�������ͷ� JSON ����ʱ
 *  һ���ͷţ����Բ���Ҫ�����ͷ�
 */
JSON_API JSON_NODE *json_create_obj(JSON *json);

/**
 * ���� json ����ʱ���� json �������(�������� [] �Ķ���)
 * @param json {JSON*} �� json_alloc / json_alloc1 ����
 * @return {JSON_NODE*} �´����Ľڵ�������ͷ� JSON ����ʱ
 *  һ���ͷţ����Բ���Ҫ�����ͷ�
 */
JSON_API JSON_NODE *json_create_array(JSON *json);

/**
 * ���� json ����ʱ���� json �ڵ����(�� tagname: JSON_NODE)
 * @param json {JSON*} �� json_alloc / json_alloc1 ����
 * @param name {const char*} json �ڵ�ı�ǩ��
 * @param value {JSON_NODE*} json �ڵ������Ϊ��ǩֵ
 * @return {JSON_NODE*} �´����Ľڵ�������ͷ� JSON ����ʱ
 *  һ���ͷţ����Բ���Ҫ�����ͷ�
 */
JSON_API JSON_NODE *json_create_node(JSON *json,
	const char *name, JSON_NODE *value);

/**
 * ���� json ����ʱ����һ���� json_create_obj �� json_create_array
 * ������ json �ڵ�����ӽڵ㣬���ӽڵ�����������½ӿڴ����Ľڵ�:
 * json_create_leaf, json_create_obj, json_create_array
 */
JSON_API void json_node_append_child(JSON *json, JSON_NODE *parent, JSON_NODE *child);

/**
 * �� json �����һ�� JSON �ڵ�ת���ַ�������
 * @param node {JSON_NODE*} json �ڵ����
 * @param buf {VSTRING*} �洢������Ļ����������ò���Ϊ��ʱ�����ڲ���
 *  �Զ�����һ�λ�������Ӧ���������Ҫ�ͷŵ����ǿպ����ڲ���ֱ�ӽ�����洢����
 * @return {VSTRING*} json �ڵ����ת�����ַ�����Ĵ洢��������
 *  �÷���ֵ��Զ�ǿգ�ʹ���߿���ͨ�� VSTRING_LEN(x) �����ж������Ƿ�Ϊ�գ�
 *  ���ص� VSTRING ָ�����Ϊ�ú����ڲ������ģ����û���������
 *  vstring_free �����ͷ�
 */
JSON_API VSTRING *json_node_build(JSON_NODE *json, VSTRING *buf);

/**
 * �� json ����ת���ַ�������
 * @param json {JSON*} json ����
 * @return {VSTRING*} json ����ת�����ַ�����Ĵ洢���������÷���ֵ��Զ�ǿգ�
 *  ʹ���߿���ͨ�� VSTRING_LEN(x) �����ж������Ƿ�Ϊ�գ����ص� VSTRING
 *  ָ�����Ϊ�ú����ڲ������ģ����û��������� vstring_free �����ͷ�
 */
JSON_API VSTRING *json_build(JSON *json);

/**
 * ��ʽ JSON ����ת�ַ���������̣����ú����ڽ� JSON ����תΪ�ַ����Ĺ����У�
 * һ��ת��һ�߽�����ͨ���ص���������������ߣ������߿����޶������޶����û�
 * ��������ʱ�����ô��������Ӧ�ڵ�JSON����ת�ɵ��ַ����ǳ���ʱ(�糬��100 MB),
 * ��Ϊ������ʽת����ʽ�����Բ�����Ҫ����һ�����ڴ�
 * @param json {JSON*} json ����
 * @param length {size_t} ��ת��Ϊ�ַ����Ĺ�����������������ȳ����ó�����ص�
 *  �û��趨�Ļص�����
 * @param callback {int (*)(JSON*, VSTRING*, void*)} �û��趨�Ļص�
 *  ���������ص��������ĵڶ�������Ϊ NULL ʱ��ʾ������ϣ�����û��ڸûص�
 *  ��ĳ�α����ú󷵻�ֵ < 0 ��ֹͣ�������
 * @param ctx {void*} callback ���������һ������
 */
JSON_API void json_building(JSON *json, size_t length,
	int (*callback)(JSON *, VSTRING *, void *), void *ctx);

#ifdef __cplusplus
}
#endif

#endif
