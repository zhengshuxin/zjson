#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "json/dbuf.h"
#include "json/vstring.h"
#include "json/json.h"

#define STR	VSTRING_STR
#define	LEN	VSTRING_LEN
#define ADDCH	VSTRING_ADDCH
#define TERM	VSTRING_TERMINATE

JSON_NODE *json_getFirstElementByTagName(JSON *json, const char *tag)
{
	ITER iter;

	foreach(iter, json) {
		JSON_NODE *node = (JSON_NODE*) iter.data;
		if (strcasecmp(tag, STR(node->ltag)) == 0) {
			return node;
		}
	}

	return NULL;
}

#if 0
void json_free_array(ARRAY *a)
{
	array_destroy(a, NULL);
}

ARRAY *json_getElementsByTagName(JSON *json, const char *tag)
{
	ITER iter;
	ARRAY *a = array_create(10);

	foreach(iter, json) {
		JSON_NODE *node = (JSON_NODE*) iter.data;
		if (strcasecmp(tag, STR(node->ltag)) == 0) {
			array_append(a, node);
		}
	}

	if (array_size(a) == 0) {
		array_destroy(a, NULL);
		return (NULL);
	}

	return (a);
}

ARRAY *json_getElementsByTags(JSON *json, const char *tags)
{
	ARGV *tokens = argv_split(tags, "/");
	ARRAY *a, *result;
	ITER iter;
	JSON_NODE *node_saved, *node;
	int   i;

	a = json_getElementsByTagName(json, tokens->argv[tokens->argc - 1]);
	if (a == NULL) {
		argv_free(tokens);
		return (NULL);
	}

	result = array_create(array_size(a));

#define	NEQ(x, y) strcasecmp((x), (y))

	foreach(iter, a) {
		node = (JSON_NODE*) iter.data;
		node_saved = node;
		i = tokens->argc - 1;
		while (i >= 0 && node->parent != NULL) {
			if (node->left_ch != 0) {
				node = node->parent;
			} else if (NEQ(tokens->argv[i], "*")
				&& NEQ(tokens->argv[i], STR(node->ltag)))
			{
				break;
			} else {
				i--;
				node = node->parent;
			}
		}
		if (i == -1)
			result->push_back(result, node_saved);
	}

	json_free_array(a);
	argv_free(tokens);

	if (array_size(result) == 0) {
		array_free(result, NULL);
		result = NULL;
	}
	return result;
}
#endif

JSON_NODE *json_create_text(JSON *json, const char *name, const char *value)
{
	JSON_NODE *node = json_node_alloc(json);

	node->ltag = vstring_strcpy(json->dbuf, node->ltag, name);
	node->text = vstring_strcpy(json->dbuf, node->text, value);
	node->type = JSON_T_STRING;
	return node;
}

JSON_NODE *json_create_bool(JSON *json, const char *name, int value)
{
	JSON_NODE *node = json_node_alloc(json);

	node->ltag = vstring_strcpy(json->dbuf, node->ltag, name);
	node->text = vstring_strcpy(json->dbuf, node->text, value == 0 ? "false" : "true");
	node->type = JSON_T_BOOL;
	return node;
}

JSON_NODE *json_create_null(JSON *json, const char *name)
{
	JSON_NODE *node = json_node_alloc(json);

	node->ltag = vstring_strcpy(json->dbuf, node->ltag, name);
	node->text = vstring_strcpy(json->dbuf, node->text, "null");
	node->type = JSON_T_NULL;
	return node;
}

JSON_NODE *json_create_int64(JSON *json, const char *name, long long value)
{
	JSON_NODE *node = json_node_alloc(json);
	char buf[32];

	snprintf(buf, sizeof(buf), "%lld", value);
	node->ltag = vstring_strcpy(json->dbuf, node->ltag, name);
	node->text = vstring_strcpy(json->dbuf, node->text, buf);
	node->type = JSON_T_NUMBER;
	return node;
}

JSON_NODE *json_create_double(JSON *json, const char *name, double value)
{
	JSON_NODE *node = json_node_alloc(json);
	char buf[32];

	snprintf(buf, sizeof(buf), "%.4f", value);
	node->ltag = vstring_strcpy(json->dbuf, node->ltag, name);
	node->text = vstring_strcpy(json->dbuf, node->text, buf);
	node->type = JSON_T_DOUBLE;
	return node;
}

JSON_NODE *json_create_array_text(JSON *json, const char *text)
{
	JSON_NODE *node = json_node_alloc(json);

	node->text = vstring_strcpy(json->dbuf, node->text, text);
	node->type = JSON_T_A_STRING;
	return node;
}

JSON_NODE *json_create_array_int64(JSON *json, long long value)
{
	JSON_NODE *node = json_node_alloc(json);
	char buf[32];

	snprintf(buf, sizeof(buf), "%lld", value);
	node->text = vstring_strcpy(json->dbuf, node->text, buf);
	node->type = JSON_T_A_NUMBER;
	return node;
}

JSON_NODE *json_create_array_double(JSON *json, double value)
{
	JSON_NODE *node = json_node_alloc(json);
	char buf[32];

	snprintf(buf, sizeof(buf), "%.4f", value);
	node->text = vstring_strcpy(json->dbuf, node->text, buf);
	node->type = JSON_T_A_DOUBLE;
	return node;
}

JSON_NODE *json_create_array_bool(JSON *json, int value)
{
	JSON_NODE *node = json_node_alloc(json);

	node->text = vstring_strcpy(json->dbuf, node->text,
			value ? "true" : "false");
	node->type = JSON_T_A_BOOL;
	return node;
}

JSON_NODE *json_create_array_null(JSON *json)
{
	JSON_NODE *node = json_node_alloc(json);

	node->text = vstring_strcpy(json->dbuf, node->text, "null");
	node->type = JSON_T_A_NULL;
	return node;
}

JSON_NODE *json_create_obj(JSON *json)
{
	JSON_NODE *node = json_node_alloc(json);

	node->left_ch  = '{';
	node->right_ch = '}';
	node->type     = JSON_T_OBJ;
	return node;
}

JSON_NODE *json_create_array(JSON *json)
{
	JSON_NODE *node = json_node_alloc(json);

	node->left_ch  = '[';
	node->right_ch = ']';
	node->type     = JSON_T_ARRAY;
	return node;
}

JSON_NODE *json_create_node(JSON *json, const char *name, JSON_NODE *value)
{
	JSON_NODE *node = json_node_alloc(json);

	node->ltag     = vstring_strcpy(json->dbuf, node->ltag, name);
	node->tag_node = value;
	node->type     = JSON_T_OBJ;
	json_node_add_child(node, value);
	return node;
}

void json_node_append_child(JSON *json, JSON_NODE *parent, JSON_NODE *child)
{
	if (parent->type != JSON_T_ARRAY
		&& parent->type != JSON_T_OBJ
		&& parent != json->root) {

		printf("%s(%d): parent's type not array or obj\r\n",
			__FUNCTION__, __LINE__);
		assert(0);
	}
	json_node_add_child(parent, child);
}

static VSTRING *json_escape_append(JSON *json, VSTRING *buf, const char *src)
{
	const unsigned char *ptr = (const unsigned char*) src;

	ADDCH(json->dbuf, buf, '"');

	while (*ptr) {
		if (*ptr == '"' || *ptr == '\\') {
			ADDCH(json->dbuf, buf, '\\');
			ADDCH(json->dbuf, buf, *ptr);
		} else if (*ptr == '\b') {
			ADDCH(json->dbuf, buf, '\\');
			ADDCH(json->dbuf, buf, 'b');
		} else if (*ptr == '\f') {
			ADDCH(json->dbuf, buf, '\\');
			ADDCH(json->dbuf, buf, 'f');
		} else if (*ptr == '\n') {
			ADDCH(json->dbuf, buf, '\\');
			ADDCH(json->dbuf, buf, 'n');
		} else if (*ptr == '\r') {
			ADDCH(json->dbuf, buf, '\\');
			ADDCH(json->dbuf, buf, 'r');
		} else if (*ptr == '\t') {
			ADDCH(json->dbuf, buf, '\\');
			ADDCH(json->dbuf, buf, 't');
		} else {
			ADDCH(json->dbuf, buf, *ptr);
		}
		ptr++;
	}

	ADDCH(json->dbuf, buf, '"');
	TERM(json->dbuf, buf);
	return buf;
}

#if 0
void json_building(JSON *json, size_t length,
	int (*callback)(JSON *, VSTRING *, void *), void *ctx)
{
	ITER iter;
	JSON_NODE *node, *prev;
	VSTRING *buf = vstring_alloc(256);

	if (json->root->left_ch > 0) {
		ADDCH(buf, json->root->left_ch);
	}

	foreach(iter, json) {
		if (VSTRING_LEN(buf) >= length && callback != NULL) {
			VSTRING_TERMINATE(buf);
			if (callback(json, buf, ctx) < 0) {
				vstring_free(buf);
				return;
			}
			VSTRING_RESET(buf);
		}

		node = (JSON_NODE*) iter.data;
		prev = json_node_prev(node);
		if (prev != NULL) {
			if ((json->flag & JSON_FLAG_ADD_SPACE))
				vstring_strcat(buf, ", ");
			else
				vstring_strcat(buf, ",");
		}

		/* 只有当标签的对应值为 JSON 对象或数组对象时 tag_node 非空 */
		if (node->tag_node != NULL) {
			if (LEN(node->ltag) > 0) {
				json_escape_append(buf, STR(node->ltag));
				ADDCH(json->dbuf, buf, ':');
				if ((json->flag & JSON_FLAG_ADD_SPACE)) {
					ADDCH(json->dbuf, buf, ' ');
				}
			}

			/* '{' or '[' */	
			if (node->left_ch != 0) {
				ADDCH(json->dbuf, buf, node->left_ch);
			}
		}

		/* 当节点有标签名时 */
		else if (LEN(node->ltag) > 0) {
			json_escape_append(buf, STR(node->ltag));
			ADDCH(json->dbuf, buf, ':');
			if ((json->flag & JSON_FLAG_ADD_SPACE)) {
				ADDCH(json->dbuf, buf, ' ');
			}

			switch (node->type & ~JSON_T_LEAF) {
			case JSON_T_NULL:
				vstring_strcat(buf, "null");
				break;
			case JSON_T_BOOL:
			case JSON_T_NUMBER:
			case JSON_T_DOUBLE:
				vstring_strcat(buf, STR(node->text));
				break;
			default:
				json_escape_append(buf, STR(node->text));
				break;
			}
		}

		/* 当节点为数组的成员时 */
		else if (LEN(node->text) > 0 && node->parent
			&& node->parent->left_ch != 0)
		{
			switch (node->type & ~JSON_T_LEAF) {
			case JSON_T_A_NULL:
				vstring_strcat(buf, "null");
				break;
			case JSON_T_A_BOOL:
			case JSON_T_A_NUMBER:
			case JSON_T_A_DOUBLE:
				vstring_strcat(buf, STR(node->text));
				break;
			default:
				json_escape_append(buf, STR(node->text));
				break;
			}
		}

		/* 当节点为没有标签名的容器(为 '{}' 或 '[]')时 */
		else if (node->left_ch != 0) {
			ADDCH(json->dbuf, buf, node->left_ch);
		}

		/*
		 * 遍历方式为前序遍历方式，即先遍历当前节点的子节点，
		 * 再遍历当前节点的子节点，最后遍历当前节点的父节点
		 */
		/* 当本节点有子节点或虽为叶节点，但该节点的下一个兄弟节点
		 * 非空时继续下一个循环过程
		 */
		if (ring_size(&node->children) > 0)
			continue;
		else if (json_node_next(node) != NULL) {
			if (node->right_ch > 0) {
				ADDCH(json->dbuf, buf, node->right_ch);
			}
			continue;
		}

		if (node->right_ch > 0) {
			ADDCH(json->dbuf, buf, node->right_ch);
		}

		/* 当本节点为叶节点且后面没有兄弟节点时，需要一级一级回溯
		 * 将父节点的分隔符添加至本叶节点尾部，直到遇到根节点或父
		 * 节点的下一个兄弟节点非空
		 */
		while (json_node_next(node) == NULL) {
			if (node->parent == json->root)
				break;

			node = node->parent;

			/* right_ch: '}' or ']' */
			if (node->right_ch != 0) {
				ADDCH(json->dbuf, buf, node->right_ch);
			}
		}
	}

	if (json->root->right_ch > 0) {
		ADDCH(json->dbuf, buf, json->root->right_ch);
	}

	VSTRING_TERMINATE(buf);
	if (VSTRING_LEN(buf) > 0 && callback != NULL) {
		if (callback(json, buf, ctx) < 0) {
			vstring_free(buf);
			return;
		}
	}

	vstring_free(buf);

	/* 将第二个参数置 NULL 表示处理完毕 */
	if (callback != NULL)
		(void) callback(json, NULL, ctx);
}

#endif

VSTRING *json_build(JSON *json)
{
	JSON_NODE *node, *prev;
	VSTRING *buf;
	ITER iter;

	buf = vstring_alloc(json->dbuf, 256);

	if (json->root->left_ch > 0) {
		ADDCH(json->dbuf, buf, json->root->left_ch);
	}

	foreach(iter, json) {
		node = (JSON_NODE*) iter.data;
		prev = json_node_prev(node);
		if (prev != NULL) {
			if ((json->flag & JSON_FLAG_ADD_SPACE)) {
				buf = vstring_strcat(json->dbuf, buf, ", ");
			} else {
				buf = vstring_strcat(json->dbuf, buf, ",");
			}
		}

		/* 只有当标签的对应值为 JSON 对象或数组对象时 tag_node 非空 */
		if (node->tag_node != NULL) {
			if (node->ltag && LEN(node->ltag) > 0) {
				buf = json_escape_append(json, buf, STR(node->ltag));
				ADDCH(json->dbuf, buf, ':');
				if ((json->flag & JSON_FLAG_ADD_SPACE)) {
					ADDCH(json->dbuf, buf, ' ');
				}
			}

			/* '{' or '[' */	
			if (node->left_ch != 0) {
				ADDCH(json->dbuf, buf, node->left_ch);
			}
		}

		/* 当节点有标签名时 */
		else if (node->ltag && LEN(node->ltag) > 0) {
			buf = json_escape_append(json, buf, STR(node->ltag));
			ADDCH(json->dbuf, buf, ':');
			if ((json->flag & JSON_FLAG_ADD_SPACE)) {
				ADDCH(json->dbuf, buf, ' ');
			}

			switch (node->type & ~JSON_T_LEAF) {
			case JSON_T_NULL:
				buf = vstring_strcat(json->dbuf, buf, "null");
				break;
			case JSON_T_BOOL:
			case JSON_T_NUMBER:
			case JSON_T_DOUBLE:
				buf = vstring_strcat(json->dbuf, buf, STR(node->text));
				break;
			default:
				buf = json_escape_append(json, buf, STR(node->text));
				break;
			}
		}

		/* 当节点为数组的成员时 */
#if 0
		else if (LEN(node->text) > 0 && node->parent
			/* 应该依据父节点类型来确定当前节点是否为数组节点
			 * && node->parent->left_ch != 0)
			 */
			&& node->parent->type == JSON_T_ARRAY)
#elif 0
		else if (node->parent && node->parent->type == JSON_T_ARRAY
			&& (LEN(node->text) > 0 || (node->type & JSON_T_A_STRING)))
#else
		else if (node->parent && node->parent->type == JSON_T_ARRAY
			&& (node->type & (JSON_T_A_NULL
			    | JSON_T_A_BOOL | JSON_T_A_NUMBER
			    | JSON_T_A_DOUBLE | JSON_T_A_STRING)))
#endif
		{
			switch (node->type & ~JSON_T_LEAF) {
			case JSON_T_A_NULL:
				buf = vstring_strcat(json->dbuf, buf, "null");
				break;
			case JSON_T_A_BOOL:
			case JSON_T_A_NUMBER:
			case JSON_T_A_DOUBLE:
				buf = vstring_strcat(json->dbuf, buf, STR(node->text));
				break;
			case JSON_T_A_STRING:
				buf = json_escape_append(json, buf, STR(node->text));
				break;
			default:
				break;
			}
		}

		/* 当节点为没有标签名的容器(为 '{}' 或 '[]')时 */
		else if (node->left_ch != 0) {
			ADDCH(json->dbuf, buf, node->left_ch);
		}

		/*
		 * 遍历方式为前序遍历方式，即先遍历当前节点的子节点，
		 * 再遍历当前节点的子节点，最后遍历当前节点的父节点
		 */
		/* 当本节点有子节点或虽为叶节点，但该节点的下一个兄弟节点
		 * 非空时继续下一个循环过程
		 */
		if (!ring_empty(&node->children)) {
			continue;
		} else if (json_node_next(node) != NULL) {
			if (node->right_ch > 0) {
				ADDCH(json->dbuf, buf, node->right_ch);
			}
			continue;
		}

		if (node->right_ch > 0) {
			ADDCH(json->dbuf, buf, node->right_ch);
		}

		/* 当本节点为叶节点且后面没有兄弟节点时，需要一级一级回溯
		 * 将父节点的分隔符添加至本叶节点尾部，直到遇到根节点或父
		 * 节点的下一个兄弟节点非空
		 */
		while (json_node_next(node) == NULL) {
			if (node->parent == json->root) {
				break;
			}

			node = node->parent;

			/* right_ch: '}' or ']' */
			if (node->right_ch != 0) {
				ADDCH(json->dbuf, buf, node->right_ch);
			}
		}
	}

	if (json->root->right_ch > 0) {
		ADDCH(json->dbuf, buf, json->root->right_ch);
	}

	TERM(json->dbuf, buf);
	return buf;
}

#if 0
VSTRING *json_node_build(JSON_NODE *node, VSTRING *buf)
{
	JSON *json = json_alloc();
	JSON_NODE *first;

	if (buf == NULL) {
		buf = vstring_alloc(256);
	}

	if (node == node->json->root && node->tag_node != NULL)
		node = node->tag_node;
	else
		json->root->left_ch = json->root->right_ch = 0;

	first = json_node_duplicate(json, node);
	json_node_add_child(json->root, first);
	json_build(json, buf);
	json_free(json);

	return buf;
}
#endif
