#include "StdAfx.h"
#include "json/dbuf.h"
#include "json/vstring.h"
#include "json/array.h"
#include "json/json.h"

#define	LEN	VSTRING_LEN
#define	STR	VSTRING_STR

const char *json_node_type(unsigned short type)
{
	if (json_node_is_string(type)) {
		return "string";
	} else if (json_node_is_number(type)) {
		return "number";
	} else if (json_node_is_bool(type)) {
		return "bool";
	} else if (json_node_is_null(type)) {
		return "null";
	} else if (json_node_is_obj(type)) {
		return "obj";
	} else if (json_node_is_array(type)) {
		return "array";
	} else {
		return "unknown";
	}
}

int json_node_is_string(unsigned short type)
{
	return (type & JSON_T_A_STRING) || (type & JSON_T_STRING);
}

int json_node_is_number(unsigned short type)
{
	return  (type & JSON_T_A_DOUBLE) || (type & JSON_T_DOUBLE) ||
		(type & JSON_T_A_NUMBER) || (type & JSON_T_NUMBER);
}

int json_node_is_bool(unsigned short type)
{
	return (type & JSON_T_A_BOOL) || (type & JSON_T_BOOL);
}

int json_node_is_null(unsigned short type)
{
	return (type & JSON_T_A_NULL) || (type & JSON_T_NULL);
}

int json_node_is_obj(unsigned short type)
{
	return (type & JSON_T_OBJ);
}

int json_node_is_array(unsigned short type)
{
	return (type & JSON_T_ARRAY);
}

JSON_NODE *json_node_alloc(JSON *json)
{
	JSON_NODE *node;

	node = (JSON_NODE*) dbuf_calloc(json->dbuf, sizeof(JSON_NODE));

	ring_init(&node->children);
	ring_init(&node->node);

	node->ltag = NULL;
	node->text = NULL;
	json->part_word = 0;
	json->node_cnt++;

	return node;
}

int json_node_delete(JSON *json, JSON_NODE *node)
{
	RING *next;
	JSON_NODE *node_next;
	int   n = 1;

	while ((next = ring_pop_head(&node->children)) != NULL) {
		node_next = ring_to_appl(next, JSON_NODE, node);
		n += json_node_delete(json, node_next);
	}

	ring_detach(&node->node);
	json->node_cnt--;

	return n;
}

void json_node_append(JSON_NODE *node1, JSON_NODE *node2)
{
	ring_append(&node1->node, &node2->node);
	node2->parent = node1->parent;
}

void json_node_add_child(JSON_NODE *parent, JSON_NODE *child)
{
	ring_prepend(&parent->children, &child->node);
	child->parent = parent;
}

JSON_NODE *json_node_parent(JSON_NODE *node)
{
	return node->parent;
}

JSON_NODE *json_node_next(JSON_NODE *node)
{
	RING *ring_ptr = ring_succ(&node->node);
	JSON_NODE *parent;

	if (ring_ptr == &node->node) {
		return NULL;
	}
	parent = node->parent;
	assert(parent != NULL);
	if (ring_ptr == &parent->children) {
		return NULL;
	}
	return ring_to_appl(ring_ptr, JSON_NODE, node);
}

JSON_NODE *json_node_prev(JSON_NODE *node)
{
	RING *ring_ptr = ring_pred(&node->node);
	JSON_NODE *parent;

	if (ring_ptr == &node->node) {
		return NULL;
	}
	parent = node->parent;
	assert(parent != NULL);
	if (ring_ptr == &parent->children) {
		return NULL;
	}

	return ring_to_appl(ring_ptr, JSON_NODE, node);
}

/************************************************************************/
/*               json 对象处理函数集                                    */
/************************************************************************/

static JSON_NODE *json_iter_head(ITER *it, JSON *json)
{
	RING *ring_ptr;

	it->dlen = -1;
	it->key  = NULL;
	it->klen = -1;

	it->i = 0;
	it->size = json->node_cnt;

	ring_ptr = ring_succ(&json->root->children);
	if (ring_ptr == &json->root->children) {
		it->ptr = it->data = NULL;
		return NULL;
	}

	it->ptr  = ring_to_appl(ring_ptr, JSON_NODE, node);
	it->data = it->ptr;
	return it->ptr;
}

static JSON_NODE *json_iter_next(ITER *it, JSON *json)
{
	RING *ring_ptr;
	struct JSON_NODE *node, *parent;

	node = (struct JSON_NODE*) it->data;

	/* 先遍历当前节点的子节点 */

	ring_ptr = ring_succ(&node->children);
	if (ring_ptr != &node->children) {
		it->i++;
		it->ptr  = ring_to_appl(ring_ptr, JSON_NODE, node);
		it->data = it->ptr;
		return it->ptr;
	}

	/* 当前节点的子节点遍历完毕，再遍历当前节点的兄弟节点 */

	parent   = json_node_parent(node);
	ring_ptr = ring_succ(&node->node);
	if (ring_ptr != &parent->children) {
		it->i++;
		it->ptr  = ring_to_appl(ring_ptr, JSON_NODE, node);
		it->data = it->ptr;
		return it->ptr;
	}

	/* 当前节点的兄弟节点遍历完毕，最后遍历当前节点的父节点的兄弟节点 */

	do {
		if (parent == json->root) {
			break;
		}

		ring_ptr = ring_succ(&parent->node);
		parent   = json_node_parent(parent);
		assert(parent);

		if (ring_ptr != &parent->children) {
			it->i++;
			it->ptr  = ring_to_appl(ring_ptr, JSON_NODE, node);
			it->data = it->ptr;
			return it->ptr;
		}
	} while (ring_ptr != &json->root->children);

	/* 遍历完所有节点 */

	it->ptr = it->data = NULL;
	return NULL;
}

static JSON_NODE *json_iter_tail(ITER *it, JSON *json)
{
	RING *ring_ptr;

	it->dlen = -1;
	it->key  = NULL;
	it->klen = -1;

	it->i = 0;
	it->size = json->node_cnt;

	ring_ptr = ring_pred(&json->root->children);
	if (ring_ptr == &json->root->children) {
		it->ptr = it->data = NULL;
		return NULL;
	}

	it->ptr  = ring_to_appl(ring_ptr, JSON_NODE, node);
	it->data = it->ptr;
	return it->ptr;
}

static JSON_NODE *json_iter_prev(ITER *it, JSON *json)
{
	RING *ring_ptr;
	struct JSON_NODE *node, *parent;

	node = (struct JSON_NODE*) it->data;

	/* 先遍历当前节点的子节点 */

	ring_ptr = ring_pred(&node->children);
	if (ring_ptr != &node->children) {
		it->i++;
		it->ptr  = ring_to_appl(ring_ptr, JSON_NODE, node);
		it->data = it->ptr;
		return it->ptr;
	}

	/* 当前节点的子节点遍历完毕，再遍历当前节点的兄弟节点 */

	parent   = json_node_parent(node);
	ring_ptr = ring_pred(&node->node);
	if (ring_ptr != &parent->children) {
		it->i++;
		it->ptr  = ring_to_appl(ring_ptr, JSON_NODE, node);
		it->data = it->ptr;
		return it->ptr;
	}

	/* 当前节点的兄弟节点遍历完毕，最后遍历当前节点的父节点的兄弟节点 */

	do {
		if (parent == json->root) {
			break;
		}
		ring_ptr = ring_pred(&parent->node);
		parent   = json_node_parent(parent);
		assert(parent);

		if (ring_ptr != &parent->children) {
			it->i++;
			it->ptr  = ring_to_appl(ring_ptr, JSON_NODE, node);
			it->data = it->ptr;
			return it->ptr;
		}
	} while (ring_ptr != &json->root->children);

	/* 遍历完所有节点 */

	it->ptr = it->data = NULL;
	return NULL;
}

JSON *json_alloc(void)
{
	JSON *json;
	DBUF *dbuf;

	dbuf = dbuf_create(8192);
	json = (JSON*) dbuf_calloc(dbuf, sizeof(JSON));

	json->dbuf = dbuf;
	json->keep = sizeof(JSON);

	json->buf  = vstring_alloc(dbuf, 128);
	json->root = json_node_alloc(json);
	/* 将根节点作为当前节点 */
	json->curr_node = json->root;

	json->root->left_ch  = '{';
	json->root->right_ch = '}';

	/* 设置状态机的状态 */
	json->status = JSON_S_ROOT;

	/* 设置迭代函数 */

	json->iter_head = json_iter_head;
	json->iter_next = json_iter_next;
	json->iter_tail = json_iter_tail;
	json->iter_prev = json_iter_prev;

	return json;
}

JSON_NODE *json_node_duplicate(JSON *json, JSON_NODE *from)
{
	JSON_NODE *child_from, *child_to, *to;
	RING_ITER iter;

	to = json_node_alloc(json);
	to->left_ch  = from->left_ch;
	to->right_ch = from->right_ch;
	to->type     = from->type;
	to->depth    = from->depth;  /* XXX? */

	to->ltag = vstring_strcpy(json->dbuf, to->ltag,
			(const char*) STR(from->ltag));
	to->text = vstring_strcpy(json->dbuf, to->text,
			(const char*) STR(from->text));

	ring_foreach(iter, &from->children) {
		child_from = ring_to_appl(iter.ptr, JSON_NODE, node);
		child_to   = json_node_duplicate(json, child_from);
		json_node_add_child(to, child_to);
		if (from->tag_node == child_from) {
			to->tag_node = child_to;
		}
	}
	return to;
}

JSON *json_create(JSON_NODE *node)
{
	JSON *json;
	JSON_NODE *first;
	DBUF *dbuf = dbuf_create(8192);

	json = (JSON*) dbuf_calloc(dbuf, sizeof(JSON));
	json->dbuf = dbuf;
	json->root = json_node_alloc(json);

	first = json_node_duplicate(json, node);
	json_node_add_child(json->root, first);

	/* 将根节点作为当前节点 */
	json->curr_node = json->root;
	/* 设置状态机的状态 */
	json->status = JSON_S_ROOT;

	json->root->left_ch  = '{';
	json->root->right_ch = '}';

	/* 设置迭代函数 */

	json->iter_head = json_iter_head;
	json->iter_next = json_iter_next;
	json->iter_tail = json_iter_tail;
	json->iter_prev = json_iter_prev;

	return json;
}

void json_foreach_init(JSON *json, JSON_NODE *node)
{
	json->root      = node;
	json->iter_head = json_iter_head;
	json->iter_next = json_iter_next;
	json->iter_tail = json_iter_tail;
	json->iter_prev = json_iter_prev;
}

void json_free(JSON *json)
{
	dbuf_destroy(json->dbuf);
}

void json_reset(JSON *json)
{
	dbuf_reset(json->dbuf, json->keep);

	json->root = json_node_alloc(json);
	json->root->left_ch  = '{';
	json->root->right_ch = '}';
	json->root->type = JSON_T_OBJ;

	json->node_cnt  = 1;
	json->curr_node = json->root;
	json->status    = JSON_S_ROOT;
	json->finish    = 0;
	json->depth     = 0;
}

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

void json_free_array(ARRAY *a)
{
	array_free(a, NULL);
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
		array_free(a, NULL);
		return NULL;
	}

	return a;
}

ARRAY *json_getElementsByTags(JSON *json, const char *tags)
{
	int   i;
	ITER iter;
	ARRAY *tokens, *a, *result;
	JSON_NODE *node_saved, *node;
	char *buf = strdup(tags), *slash, *ptr;

	ptr = buf;
	tokens = array_create(10);
	while (*ptr) {
		slash = strchr(ptr, '/');
		if (slash) {
			*slash++ = 0;
			array_append(tokens, ptr);
			ptr = slash;
		} else {
			array_append(tokens, ptr);
			break;
		}
	}

	i = array_size(tokens);
	ptr = (char*) array_index(tokens, i - 1);
	a = json_getElementsByTagName(json, ptr);
	if (a == NULL) {
		array_free(tokens, NULL);
		free(buf);
		return (NULL);
	}

	result = array_create(array_size(a));

#define	NEQ(x, y) strcasecmp((x), (y))

	foreach(iter, a) {
		node = (JSON_NODE*) iter.data;
		node_saved = node;
		i = array_size(tokens) - 1;
		while (i >= 0 && node->parent != NULL) {
			ptr = (char*) array_index(tokens, i);
			if (node->left_ch != 0) {
				node = node->parent;
			} else if (NEQ(ptr, "*") && NEQ(ptr, STR(node->ltag))) {
				break;
			} else {
				node = node->parent;
				i--;
			}
		}
		if (i < 0) {
			array_append(result, node_saved);
		}
	}

	json_free_array(a);
	array_free(tokens, NULL);
	free(buf);

	if (array_size(result) == 0) {
		array_free(result, NULL);
		result = NULL;
	}

	return result;
}
