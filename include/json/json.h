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
	VSTRING   *ltag;		/**< 标签名 */
	VSTRING   *text;		/**< 当节点为叶节点时该文本内容非空 */
	JSON_NODE *tag_node;		/**< 当标签值为 json 节点时此项非空 */
	JSON_NODE *parent;		/**< 父节点 */
	RING       children;		/**< 子节点集合 */
	RING       node;		/**< 当前节点 */
	unsigned int   depth:30;	/**< 当前节点的深度 */
	unsigned int   quoted:1;	/**< 该节点是否被引号包含 */
	unsigned short type;		/**< 节点类型 */
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

	unsigned char left_ch;	/**< 本节点的第一个字符: { or [ */
	unsigned char right_ch;	/**< 本节点的最后一个字符: } or ] */
};

enum {
	JSON_S_ROOT,		/**< 根节点 */
	JSON_S_OBJ,		/**< 标签对象值 */
	JSON_S_MEMBER,
	JSON_S_ARRAY,		/**< json 节点 array */
	JSON_S_ELEMENT,
	JSON_S_PAIR,		/**< name:value pair */
	JSON_S_NEXT,		/**< 下一个节点 */
	JSON_S_TAG,		/**< 对象标签名 */
	JSON_S_VALUE,		/**< 节点值处理过程 */
	JSON_S_COLON,		/**< 冒号 : */
	JSON_S_STRING,
	JSON_S_STREND
};

struct JSON {
	unsigned depth;		/**< 当前 Json 对象的最大深度 */
	int   node_cnt;		/**< 节点总数, 包括 root 节点 */
	JSON_NODE *root;	/**< json 根节点 */
	int   finish;		/**< 是否分析结束 */
	unsigned flag;		/**< 标志位 */
#define	JSON_FLAG_PART_WORD	(1 << 0)	/**< 是否兼容半个汉字 */
#define JSON_FLAG_ADD_SPACE	(1 << 1)	/**< 创建 json 时是否添空格 */

	/* public: for iterator, 通过 foreach 可以列出所有子节点 */

	/* 取迭代器头函数 */
	JSON_NODE *(*iter_head)(ITER*, JSON*);
	/* 取迭代器下一个函数 */
	JSON_NODE *(*iter_next)(ITER*, JSON*);
	/* 取迭代器尾函数 */
	JSON_NODE *(*iter_tail)(ITER*, JSON*);
	/* 取迭代器上一个函数 */
	JSON_NODE *(*iter_prev)(ITER*, JSON*);

	/* private */

	int   status;		/**< 状态机当前解析状态 */

	JSON_NODE *curr_node;	/**< 当前正在处理的 json 节点 */
	DBUF  *dbuf;		/**< 会话内存池对象 */
	size_t keep;		/**< 调用 dbuf_reset 时希望保留的内存长度 */

	VSTRING *buf;
	unsigned char quote;	/**< 非 0 表示 ' 或 " */
	unsigned backslash:1;	/**< 转义字符 \ */
	unsigned part_word:1;	/**< 半个汉字的情况处理标志位 */
};

/*----------------------------- in json.c -----------------------------*/

/**
 * 将 Json 节点的整形值转为字符串描述
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
 * 创建一个 json 节点
 * @param json {JSON*} json 对象
 * @return {JSON_NODE*} json 节点对象
 */
JSON_API JSON_NODE *json_node_alloc(JSON *json);

/**
 * 将某个 json 节点及其子节点从 json 对象中删除, 并释放该节点及其子节点
 * 所占空间函数来释放该 json 节点所占内存
 * @param json {JSON*} json 对象
 * @param node {JSON_NODE*} json 节点
 * @return {int} 返回删除的节点个数
 */
JSON_API int json_node_delete(JSON *json, JSON_NODE *node);

/**
 * 向某个 json 节点添加兄弟节点(该兄弟节点必须是独立的 json 节点)
 * @param node1 {JSON_NODE*} 向本节点添加 json 节点
 * @param node2 {JSON_NODE*} 新添加的兄弟 json 节点
 */
JSON_API void json_node_append(JSON_NODE *node1, JSON_NODE *node2);

/**
 * 将某个 json 节点作为子节点加入某父 json 节点中
 * @param parent {JSON_NODE*} 父节点
 * @param child {JSON_NODE*} 子节点
 */
JSON_API void json_node_add_child(JSON_NODE *parent, JSON_NODE *child);

/**
 * 将一个 JSON 对象的 JSON 节点复制至 JSON 对象中的一个 JSON 节点中，并返回
 * 目标新创建的 JSON 节点
 * @param json {JSON*} 目标 JSON 对象
 * @param from {JSON_NODE*} 源 JSON 对象的一个 JSON 节点
 * @return {JSON_NODE*} 返回非空对象指针
 */
JSON_API JSON_NODE *json_node_duplicate(JSON *json, JSON_NODE *from);

/**
 * 获得某个 json 节点的父节点
 * @param node {JSON_NODE*} json 节点
 * @return {JSON_NODE*} 父节点, 如果为 NULL 则表示其父节点不存在
 */
JSON_API JSON_NODE *json_node_parent(JSON_NODE *node);

/**
 * 获得某个 json 节点的后一个兄弟节点
 * @param node {JSON_NODE*} json 节点
 * @return {JSON_NODE*} 给定 json 节点的后一个兄弟节点, 若为NULL则表示不存在
 */
JSON_API JSON_NODE *json_node_next(JSON_NODE *node);

/**
 * 获得某个 json 节点的前一个兄弟节点
 * @param node {JSON_NODE*} json 节点
 * @return {JSON_NODE*} 给定 json 节点的前一个兄弟节点, 若为NULL则表示不存在
 */
JSON_API JSON_NODE *json_node_prev(JSON_NODE *node);

/**
 * 创建一个 json 对象
 * @return {JSON*} 新创建的 json 对象
 */
JSON_API JSON *json_alloc(void);

/**
 * 根据一个 JSON 对象的一个 JSON 节点创建一个新的 JSON 对象
 * @param node {JSON_NODE*} 源 JSON 对象的一个 JSON 节点
 * @return {JSON*} 新创建的 JSON 对象
 */
JSON_API JSON *json_create(JSON_NODE *node);

/**
 * 将某一个 JSON_NODE 节点作为一个 json 对象的根节点，
 * 从而可以方便地遍历出该节点的各级子节点(在遍历过程中的所有
 * 节点不含本节点自身)，该遍历方式有别于单独
 * 遍历某一个 JSON_NODE 节点时仅能遍历其一级子节点的情形
 * @param json {JSON*} json 对象
 * @param node {JSON_NODE*} JSON_NODE 节点
 */
JSON_API void json_foreach_init(JSON *json, JSON_NODE *node);

/**
 * 释放一个 json 对象, 同时释放该对象里容纳的所有 json 节点
 * @param json {JSON*} json 对象
 */
JSON_API void json_free(JSON *json);

/**
 * 重置 json 解析器对象
 * @param json {JSON*} json 对象
 */
JSON_API void json_reset(JSON *json);

/**
 * 从 json 对象中获得第一个与所给标签名相同的 json 节点
 * @param json {JSON*} json 对象
 * @param tag {const char*} 标签名称
 * @return {JSON_NODE*} 符合条件的 json 节点, 若返回 NULL 则
 *  表示没有符合条件的 json 节点
 */
JSON_API JSON_NODE *json_getFirstElementByTagName(JSON *json, const char *tag);

/**
 * 释放由 json_getElementsByTagName, json_getElementsByName,
 * 等函数返回的动态数组对象, 因为该动态数组中的
 * 元素都是 JSON 对象中元素的引用, 所以释放掉该动态数组后, 只要 JSON
 * 对象不释放, 则原来存于该数组中的元素依然可以使用.
 * 但并不释放里面的 xml 节点元素
 * @param a {ARRAY*} 动态数组对象
 */
JSON_API void json_free_array(ARRAY *a);

/**
 * 从 json 对象中获得所有的与所给标签名相同的 json 节点的集合
 * @param json {JSON*} json 对象
 * @param tag {const char*} 标签名称
 * @return {ARRAY*} 符合条件的 json 节点集合, 存于 动态数组中, 若返回 NULL 则
 *  表示没有符合条件的 json 节点, 非空值需要调用 json_free_array 释放
 */
JSON_API ARRAY *json_getElementsByTagName(JSON *json, const char *tag);

/**
 * 从 json 对象中获得所有的与给定多级标签名相同的 json 节点的集合
 * @param json {JSON*} json 对象
 * @param tags {const char*} 多级标签名，由 '/' 分隔各级标签名，如针对 json 数据：
 *  { 'root': [
 *      'first': { 'second': { 'third': 'test1' } },
 *      'first': { 'second': { 'third': 'test2' } },
 *      'first': { 'second': { 'third': 'test3' } }
 *    ]
 *  }
 *  可以通过多级标签名：root/first/second/third 一次性查出所有符合条件的节点
 * @return {ARRAY*} 符合条件的 json 节点集合, 存于 动态数组中, 若返回 NULL 则
 *  表示没有符合条件的 json 节点, 非空值需要调用 json_free_array 释放
 */
JSON_API ARRAY *json_getElementsByTags(JSON *json, const char *tags);

/*------------------------- in json_parser.c ---------------------------*/

/**
 * 解析 json 数据, 并持续地自动生成 json 节点树
 * @param json {JSON*} json 对象
 * @param data {const char*} 以 '\0' 结尾的数据字符串, 可以是完整的 json 数据;
 *  也可以是不完整的 json 数据, 允许循环调用此函数, 将不完整数据持续地输入; 该参数
 *  若为 NULL，则直接返回空串地址，因此禁止为 NULL
 * @return {const char*} 当解析结束后，该返回值表示剩余数据的指针地址
 */
JSON_API const char* json_update(JSON *json, const char *data);

/**
 * 判断 JSON 解析是否完成
 * @param json {JSON*} json 对象
 * @return {int} 返回非 0 值表示解析完成，否则表示未完成
 */
JSON_API int json_finish(JSON *json);

/*------------------------- in json_builder.c ----------------------------*/

/**
 * 构建 json 对象时创建 json 叶节点
 * @param json {JSON*} 由 json_alloc / json_alloc1 创建
 * @param name {const char*} 标签名，非空
 * @param value {const char*} 标签值，非空
 * @return {JSON_NODE*} 新创建的节点对象，在释放 JSON 对象时
 *  一起被释放，所以不需要单独释放
 */
JSON_API JSON_NODE *json_create_text(JSON *json, const char *name, const char *value);
#define json_create_leaf json_create_text

/**
 * 构建 json 对象时创建 json 布尔类型的叶节点
 * @param json {JSON*} 由 json_alloc / json_alloc1 创建
 * @param name {const char*} 标签名，非空
 * @param value {int} 布尔类型值
 * @return {JSON_NODE*} 新创建的节点对象，在释放 JSON 对象时
 *  一起被释放，所以不需要单独释放
 */
JSON_API JSON_NODE *json_create_bool(JSON *json, const char *name, int value);

/**
 * 构造 json 对象时创建 json null 类型的叶节点
 * @param json {JSON*} 由 json_alloc / json_alloc1 创建
 * @param name {const char*} 标签名，非空
 * @return {JSON_NODE*} 新创建的节点对象，在释放 JSON 对象时
 *  一起被释放，所以不需要单独释放
 */
JSON_API JSON_NODE *json_create_null(JSON *json, const char *name);

/**
 * 构建 json 对象时创建 json int 类型的叶节点
 * @param json {JSON*} 由 json_alloc / json_alloc1 创建
 * @param name {const char*} 标签名，非空
 * @param value {int64} 有符号整形值
 * @return {JSON_NODE*} 新创建的节点对象，在释放 JSON 对象时
 *  一起被释放，所以不需要单独释放
 */
JSON_API JSON_NODE *json_create_int64(JSON *json, const char *name, long long value);

/**
 * 构建 json 对象时创建 json double 类型的叶节点
 * @param json {JSON*} 由 json_alloc / json_alloc1 创建
 * @param name {const char*} 标签名，非空
 * @param value {double} 有符号整形值
 * @return {JSON_NODE*} 新创建的节点对象，在释放 JSON 对象时
 *  一起被释放，所以不需要单独释放
 */
JSON_API JSON_NODE *json_create_double(JSON *json,
	const char *name, double value);

/**
 * 构建 json 对象的字符串节点，按 json 规范，该节点只能加入至数组对象中
 * @param json {JSON*} 由 json_alloc / json_alloc1 创建
 * @param text {const char*}
 * @return {JSON_NODE*} 新创建的节点对象，在释放 JSON 对象时
 *  一起被释放，所以不需要单独释放
 */
JSON_API JSON_NODE *json_create_array_text(JSON *json, const char *text);

/**
 * 构建 json 对象的数值节点，按 json 规范，该节点只能加入至数组对象中
 * @param json {JSON*} 由 json_alloc / json_alloc1 创建
 * @param text {int64}
 * @return {JSON_NODE*} 新创建的节点对象，在释放 JSON 对象时
 *  一起被释放，所以不需要单独释放
 */
JSON_API JSON_NODE *json_create_array_int64(JSON *json, long long value);
/**
 * 构建 json 对象的数值节点，按 json 规范，该节点只能加入至数组对象中
 * @param json {JSON*} 由 json_alloc / json_alloc1 创建
 * @param text {double}
 * @return {JSON_NODE*} 新创建的节点对象，在释放 JSON 对象时
 *  一起被释放，所以不需要单独释放
 */
JSON_API JSON_NODE *json_create_array_double(JSON *json, double value);

/**
 * 构建 json 对象的布尔节点，按 json 规范，该节点只能加入至数组对象中
 * @param json {JSON*} 由 json_alloc / json_alloc1 创建
 * @param value {int} 非 0 表示 true，否则表示 false
 * @return {JSON_NODE*} 新创建的节点对象，在释放 JSON 对象时
 *  一起被释放，所以不需要单独释放
 */
JSON_API JSON_NODE *json_create_array_bool(JSON *json, int value);

/**
 * 构建 json 对象的 null 节点，按 json 规范，该节点只能加入至数组对象中
 * @param json {JSON*} 由 json_alloc / json_alloc1 创建
 * @return {JSON_NODE*} 新创建的节点对象，在释放 JSON 对象时
 *  一起被释放，所以不需要单独释放
 */
JSON_API JSON_NODE *json_create_array_null(JSON *json);

/**
 * 构建 json 对象时创建 json 对象(即仅包含 {} 的对象)
 * @param json {JSON*} 由 json_alloc / json_alloc1 创建
 * @return {JSON_NODE*} 新创建的节点对象，在释放 JSON 对象时
 *  一起被释放，所以不需要单独释放
 */
JSON_API JSON_NODE *json_create_obj(JSON *json);

/**
 * 构建 json 对象时创建 json 数组对象(即仅包含 [] 的对象)
 * @param json {JSON*} 由 json_alloc / json_alloc1 创建
 * @return {JSON_NODE*} 新创建的节点对象，在释放 JSON 对象时
 *  一起被释放，所以不需要单独释放
 */
JSON_API JSON_NODE *json_create_array(JSON *json);

/**
 * 构建 json 对象时创建 json 节点对象(即 tagname: JSON_NODE)
 * @param json {JSON*} 由 json_alloc / json_alloc1 创建
 * @param name {const char*} json 节点的标签名
 * @param value {JSON_NODE*} json 节点对象作为标签值
 * @return {JSON_NODE*} 新创建的节点对象，在释放 JSON 对象时
 *  一起被释放，所以不需要单独释放
 */
JSON_API JSON_NODE *json_create_node(JSON *json,
	const char *name, JSON_NODE *value);

/**
 * 构建 json 对象时，向一个由 json_create_obj 或 json_create_array
 * 创建的 json 节点添加子节点，该子节点可以是由如下接口创建的节点:
 * json_create_leaf, json_create_obj, json_create_array
 */
JSON_API void json_node_append_child(JSON *json, JSON_NODE *parent, JSON_NODE *child);

/**
 * 将 json 对象的一个 JSON 节点转成字符串内容
 * @param node {JSON_NODE*} json 节点对象
 * @param buf {VSTRING*} 存储结果集的缓冲区，当该参数为空时则函数内部会
 *  自动分配一段缓冲区，应用用完后需要释放掉；非空函数内部会直接将结果存储其中
 * @return {VSTRING*} json 节点对象转换成字符串后的存储缓冲区，
 *  该返回值永远非空，使用者可以通过 VSTRING_LEN(x) 宏来判断内容是否为空，
 *  返回的 VSTRING 指针如果为该函数内部创建的，则用户名必须用
 *  vstring_free 进行释放
 */
JSON_API VSTRING *json_node_build(JSON_NODE *json, VSTRING *buf);

/**
 * 将 json 对象转成字符串内容
 * @param json {JSON*} json 对象
 * @return {VSTRING*} json 对象转换成字符串后的存储缓冲区，该返回值永远非空，
 *  使用者可以通过 VSTRING_LEN(x) 宏来判断内容是否为空，返回的 VSTRING
 *  指针如果为该函数内部创建的，则用户名必须用 vstring_free 进行释放
 */
JSON_API VSTRING *json_build(JSON *json);

/**
 * 流式 JSON 对象转字符串处理过程，即该函数在将 JSON 对象转为字符串的过程中，
 * 一边转换一边将数据通过回调函数输出给调用者，调用者可以限定长度限定调用回
 * 调函数的时机；该处理过程适应于当JSON对象转成的字符串非常长时(如超过100 MB),
 * 因为采用流式转换方式，所以并不需要分配一个大内存
 * @param json {JSON*} json 对象
 * @param length {size_t} 在转换为字符串的过程中如果缓冲区长度超过该长度则回调
 *  用户设定的回调函数
 * @param callback {int (*)(JSON*, VSTRING*, void*)} 用户设定的回调
 *  函数，当回调函数给的第二个参数为 NULL 时表示处理完毕；如果用户在该回调
 *  的某次被调用后返回值 < 0 则停止处理过程
 * @param ctx {void*} callback 函数的最后一个参数
 */
JSON_API void json_building(JSON *json, size_t length,
	int (*callback)(JSON *, VSTRING *, void *), void *ctx);

#ifdef __cplusplus
}
#endif

#endif
