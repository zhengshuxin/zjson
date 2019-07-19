#include "StdAfx.h"
#include "json/dbuf.h"
#include "json/vstring.h"
#include "json/json.h"

#include "stringops.h"

#define	LEN	VSTRING_LEN
#define	STR	VSTRING_STR
#define ADDCH	VSTRING_ADDCH
#define TERM	VSTRING_TERMINATE
#define RESET	VSTRING_RESET

#define IS_QUOTE(x) ((x) == '\"' || (x) == '\'')
#define IS_SPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')
#define SKIP_WHILE(cond, ptr) { while(*(ptr) && (cond)) (ptr)++; }
#define SKIP_SPACE(ptr) { while(IS_SPACE(*(ptr))) (ptr)++; }

static const char *json_root(JSON *json, const char *data)
{
	SKIP_WHILE(*data != '{' && *data != '[', data);
	if (*data == 0) {
		return data;
	}

	if (*data == '{') {
		json->root->left_ch  = '{';
		json->root->right_ch = '}';
		json->status         = JSON_S_MEMBER;
		json->root->type     = JSON_T_OBJ;
	} else {
		json->root->left_ch  = '[';
		json->root->right_ch = ']';
		json->status         = JSON_S_ELEMENT;
		json->root->type     = JSON_T_ARRAY;
	}

	data++;

	json->curr_node = json->root;
	json->depth     = json->depth;

	return data;
}

/* �����ڵ����ֵ�������ҵ� '{' �� '[' */

static const char *json_obj(JSON *json, const char *data)
{
	JSON_NODE *obj;

	SKIP_SPACE(data);
	if (*data == 0) {
		return data;
	}

	/* �������� '{}' �ӽڵ� */

	obj = json_node_alloc(json);
	obj->type  = JSON_T_OBJ;
	obj->depth = json->curr_node->depth + 1;
	if (obj->depth > json->depth)
		json->depth = obj->depth;

	/* ���� json �ڵ����ǰ׺�Ĳ�ͬ����¼��ͬ�Ķ����׺ */
	obj->left_ch  = '{';
	obj->right_ch = '}';

	json_node_add_child(json->curr_node, obj);

	if (LEN(json->curr_node->ltag) > 0) {
		json->curr_node->tag_node = obj;
	}

	json->curr_node = obj;
	json->status    = JSON_S_MEMBER;

	return data;
}

static const char *json_member(JSON *json, const char *data)
{
	/* ����������������ڵ�ĳ�Ա���� */
	JSON_NODE *member = json_node_alloc(json);

	member->type  = JSON_T_MEMBER;
	member->depth = json->curr_node->depth + 1;
	if (member->depth > json->depth) {
		json->depth = member->depth;
	}

	json_node_add_child(json->curr_node, member);

	/* ���ó�Ա������Ϊ��ǰ JSON �����ڵ� */
	json->curr_node = member;
	json->status    = JSON_S_PAIR;

	return data;
}

/* �����ڵ�ı�ǩ���ƣ��ڵ�����û�б�ǩ����Ҷ�ڵ�û�� { } [ ] �ָ��� */

static const char *json_pair(JSON *json, const char *data)
{
	JSON_NODE *parent = json_node_parent(json->curr_node);

	SKIP_SPACE(data);
	if (*data == 0) {
		return data;
	}

	assert(parent);

	/* �����ǰ�ַ�Ϊ���ڵ���ҷָ��������ʾ���ڵ���� */
	if (*data == parent->right_ch) {
		data++;  /* ȥ�����ڵ���ҷָ��� */
		if (parent == json->root) {
			/* ������ڵ�������������� json ������� */
			json->finish = 1;
			return data;
		}
		/* �������ڵ� */
		json->curr_node = parent;
		/* ��ѯ���ڵ����һ���ֵܽڵ� */
		json->status = JSON_S_NEXT;
		return data;
	}

	/* Ϊ '{' �� '[' ʱ˵�������˵�ǰ�ڵ���ӽڵ� */
	if (*data == '{') {
		data++;
		json->status = JSON_S_OBJ;
		return data;
	} else if (*data == '[') {
		data++;
		json->status = JSON_S_ARRAY;
		return data;
	}

	/* �����ǩ��ǰ�����ţ���¼�¸����� */
	if (IS_QUOTE(*data) && json->quote == 0) {
		json->quote = *data++;
	}

	json->curr_node->type = JSON_T_PAIR;
	json->status = JSON_S_TAG;

	return data;
}

/* �����ڵ�ı�ǩ���ƣ��ڵ�����û�б�ǩ����Ҷ�ڵ�û�� { } [ ] �ָ��� */

static const char *json_tag(JSON *json, const char *data)
{
	JSON_NODE *node = json->curr_node;
	int   ch;

	RESET(json->buf);

	while ((ch = *data) != 0) {
		/* ���ǰ�������ţ�����Ҫ�ҵ���β���� */
		if (json->quote) {
			if (json->backslash) {
				if (ch == 'b') {
					ADDCH(json->dbuf, json->buf, '\b');
				} else if (ch == 'f') {
					ADDCH(json->dbuf, json->buf, '\f');
				} else if (ch == 'n') {
					ADDCH(json->dbuf, json->buf, '\n');
				} else if (ch == 'r') {
					ADDCH(json->dbuf, json->buf, '\r');
				} else if (ch == 't') {
					ADDCH(json->dbuf, json->buf, '\t');
				} else {
					ADDCH(json->dbuf, json->buf, ch);
				}
				json->backslash = 0;
			}

			/* ��Ϊ˫�ֽں���ʱ����һ���ֽ�Ϊ�ĸ�λΪ 1��
			 * �ڶ����ֽ�Ϊ 92��������ת���ַ���ͬ
			 */
			else if (ch == '\\') {
				/* ���������ֵ����� */
				if (json->part_word) {
					ADDCH(json->dbuf, json->buf, ch);
					json->part_word = 0;
				} else {
					json->backslash = 1;
				}
			} else if (ch == json->quote) {
				JSON_NODE *parent;

				parent = json_node_parent(node);

				assert(parent);

				/* ���������ӽڵ�����Ϊ�������ַ�������� */
				if (parent->left_ch == '[') {
					json->status = JSON_S_NEXT;
				}

				/* ��ǩֵ������������һ����Ҫ�ҵ�ð�� */
				else {
					json->status = JSON_S_COLON;
				}

				/* ���ڷ�����ǩ����������Ҫ�� quote �� 0��
				 * �����ڷ�����ǩֵʱ�����Ը��ø� quote ����,
				 * ������� 0�������ŷ�����ǩֵ����
				 */
				json->quote     = 0;
				json->part_word = 0;
				data++;
				break;
			}

			/* �Ƿ���ݺ�������Ϊת��� '\' ����� */
			else if ((json->flag & JSON_FLAG_PART_WORD)) {
				ADDCH(json->dbuf, json->buf, ch);

				/* ���������ֵ����� */
				if (json->part_word) {
					json->part_word = 0;
				} else if (ch < 0) {
					json->part_word = 1;
				}
			} else {
				ADDCH(json->dbuf, json->buf, ch);
			}
		}

		/* ������ǩ��ǰû�����ŵ���� */

		else if (json->backslash) {
			ADDCH(json->dbuf, json->buf, ch);
			json->backslash = 0;
		}

		/* ��Ϊ˫�ֽں���ʱ����һ���ֽ�Ϊ�ĸ�λΪ 1��
		 * �ڶ����ֽ�Ϊ 92��������ת���ַ���ͬ
		 */
		else if (ch == '\\') {
			/* ���������ֵ����� */
			if (json->part_word) {
				ADDCH(json->dbuf, json->buf, ch);
				json->part_word = 0;
			} else {
				json->backslash = 1;
			}
		} else if (IS_SPACE(ch) || ch == ':') {
			/* ��ǩ��������������һ����Ҫ�ҵ�ð�� */
			json->status    = JSON_S_COLON;
			json->part_word = 0;
			break;
		}

		/* �Ƿ���ݺ�������Ϊת��� '\' ����� */
		else if ((json->flag & JSON_FLAG_PART_WORD)) {
			ADDCH(json->dbuf, json->buf, ch);

			/* ���������ֵ����� */
			if (json->part_word) {
				json->part_word = 0;
			} else if (ch < 0) {
				json->part_word = 1;
			}
		} else {
			ADDCH(json->dbuf, json->buf, ch);
		}
		data++;
	}

	/* �����ǩ���ǿգ�����Ҫ��֤�� 0 ��β */
	if (LEN(json->buf) > 0) {
		node->ltag = vstring_memcat(json->dbuf, node->ltag,
			(const char*) STR(json->buf), LEN(json->buf));
		TERM(json->dbuf, node->ltag);
		RESET(json->buf);
	}

	return data;
}

/* һֱ�鵽ð��Ϊֹ��Ȼ���л���������ǩֵ���� */

static const char *json_colon(JSON *json, const char *data)
{
	SKIP_SPACE(data);
	if (*data == 0) {
		return data;
	}

	if (*data != ':') {
		data++;
		return data;
	}

	data++;

	/* ��һ��������ǩ������Ӧ�ı�ǩֵ���п���Ϊ�ַ�����
	 * Ҳ�п���Ϊ�ӽڵ����
	 */
	json->status = JSON_S_VALUE;

	return data;
}

static const char *json_array(JSON *json, const char *data)
{
	JSON_NODE *array;

	SKIP_SPACE(data);
	if (*data == 0) {
		return data;
	}

	/* ����������� */
	array = json_node_alloc(json);
	array->left_ch  = '[';
	array->right_ch = ']';
	array->type     = JSON_T_ARRAY;
	array->depth    = json->curr_node->depth + 1;
	if (array->depth > json->depth) {
		json->depth = array->depth;
	}

	json_node_add_child(json->curr_node, array);

	if (LEN(json->curr_node->ltag) > 0) {
		json->curr_node->tag_node = array;
	}

	json->curr_node = array;
	json->status    = JSON_S_ELEMENT;

	/* ���������Ϊ�գ���ֱ�Ӳ�ѯ���ֵܽڵ� */
	if (*data == ']') {
		json->status = JSON_S_NEXT;
		data++;
	}

	return data;
}

static const char *json_element(JSON *json, const char *data)
{
	/* ���������Ա���� */
	JSON_NODE *element;

	SKIP_SPACE(data);
	if (*data == 0) {
		return data;
	}

	if (*data == '{') {
		data++;
		json->status = JSON_S_OBJ;
		return data;
	} else if (*data == '[') {
		data++;
		json->status = JSON_S_ARRAY;
		return data;
	}

	element        = json_node_alloc(json);
	element->type  = JSON_T_ELEMENT;
	element->depth = json->curr_node->depth + 1;
	if (element->depth > json->depth) {
		json->depth = element->depth;
	}

	json_node_add_child(json->curr_node, element);

	/* ���������Ա������Ϊ��ǰ JSON �����ڵ� */
	json->curr_node = element;
	json->status    = JSON_S_VALUE;

	return data;
}

/* ������ǩֵ����ֵ�п����Ǵ��ı�(���ýڵ�ΪҶ�ڵ�)��Ҳ�п�����һ���ӽڵ� */

static const char *json_value(JSON *json, const char *data)
{
	SKIP_SPACE(data);
	if (*data == 0) {
		return data;
	}

	/* Ϊ '{' �� '[' ʱ˵�������˵�ǰ�ڵ���ӽڵ� */
	if (*data == '{') {
		data++;
		json->status = JSON_S_OBJ;
	} else if (*data == '[') {
		data++;
		json->status = JSON_S_ARRAY;
	}

	/* ����һ����Щ���ݸ�ʽΪ "xxx: ," �ķ�ʽ */
	else if (*data == ',' || *data == ';') {
		data++;
		/* �л�����ѯ�ýڵ���ֵܽڵ�Ĺ��� */
		json->status = JSON_S_NEXT;
	}

	/* ˵����ǩ������ı�ǩֵΪ�ַ��������� */
	/* �����ǩֵǰ�����ţ���¼�¸����� */
	else if (IS_QUOTE(*data)) {
		json->quote  = *data++;
		json->status = JSON_S_STRING;
	} else {
		json->status = JSON_S_STRING;
	}

	json->curr_node->type = JSON_T_LEAF;
	return data;
}

static const char *json_string(JSON *json, const char *data)
{
	JSON_NODE *node = json->curr_node;
	int   ch;

	/* Bugfix: ���ֹ�ַ�����ʼ����Ϊ�ո�ʱ��������ע����������߼� */
#if 0
	/* ���ı�����Ϊ 0 ʱ��������Ϊ��δ������Ч���ַ� */

	if (LEN(node->text) == 0) {
		/* �ȹ��˿�ͷû�õĿո� */
		//SKIP_SPACE(data);
		if (*data == 0)
			return data;
	}
#endif

	RESET(json->buf);

	/* ˵�����ڵ���Ҷ�ڵ� */

	while ((ch = *data) != 0) {
		/* �����ʼ�����ţ�����Ҫ�Ը�������Ϊ��β�� */
		if (json->quote) {
			if (json->backslash) {
				if (ch == 'b') {
					ADDCH(json->dbuf, json->buf, '\b');
				} else if (ch == 'f') {
					ADDCH(json->dbuf, json->buf, '\f');
				} else if (ch == 'n') {
					ADDCH(json->dbuf, json->buf, '\n');
				} else if (ch == 'r') {
					ADDCH(json->dbuf, json->buf, '\r');
				} else if (ch == 't') {
					ADDCH(json->dbuf, json->buf, '\t');
				} else {
					ADDCH(json->dbuf, json->buf, ch);
				}
				json->backslash = 0;
			}

			/* ��Ϊ˫�ֽں���ʱ����һ���ֽ�Ϊ�ĸ�λΪ 1��
			 * �ڶ����ֽ��п���Ϊ 92��������ת���ַ���ͬ
			 */
			else if (ch == '\\') {
				/* ���������ֵ���������ǰһ���ֽ���ǰ
				 * ������֣���ǰ��ת���������������
				 */
				if (json->part_word) {
					ADDCH(json->dbuf, json->buf, ch);
					json->part_word = 0;
				} else {
					json->backslash = 1;
				}
			} else if (ch == json->quote) {
				/* �Խڵ��ֵ�����뱣���� quote ֵ���Ա�������
				 * ��ͬ��ֵ���ͣ�bool, null, number, string
				 * json->quote = 0;
				 */

				/* �л�����ѯ�ýڵ���ֵܽڵ�Ĺ��� */
				json->status    = JSON_S_STREND;
				json->part_word = 0;
				data++;
				break;
			}

			/* �Ƿ���ݺ�������Ϊת��� '\' ����� */
			else if ((json->flag & JSON_FLAG_PART_WORD)) {
				ADDCH(json->dbuf, json->buf, ch);

				/* ��ǰһ���ֽ�Ϊǰ������֣���ǰ�ֽ�
				 * Ϊ�������֣�����Ϊһ�������ĺ���
				 */
				if (json->part_word) {
					json->part_word = 0;
				}

				/* ǰһ���ֽڷ�ǰ��������ҵ�ǰ�ֽڸ�λ
				 * Ϊ 1���������ǰ�ֽ�Ϊǰ�������
				 */
				else if (ch < 0) {
					json->part_word = 1;
				}
			} else {
				ADDCH(json->dbuf, json->buf, ch);
			}
		} else if (json->backslash) {
			ADDCH(json->dbuf, json->buf, ch);
			json->backslash = 0;
		} else if (ch == '\\') {
			if (json->part_word) {
				ADDCH(json->dbuf, json->buf, ch);
				json->part_word = 0;
			} else {
				json->backslash = 1;
			}
		} else if (IS_SPACE(ch) || ch == ',' || ch == ';'
			|| ch == '}' || ch == ']') {

			/* �л�����ѯ�ýڵ���ֵܽڵ�Ĺ��� */
			json->status = JSON_S_STREND;
			break;
		}

		/* �Ƿ���ݺ�������Ϊת��� '\' ����� */
		else if ((json->flag & JSON_FLAG_PART_WORD)) {
			ADDCH(json->dbuf, json->buf, ch);

			/* ���������ֵ����� */
			if (json->part_word) {
				json->part_word = 0;
			} else if (ch < 0) {
				json->part_word = 1;
			}
		} else {
			ADDCH(json->dbuf, json->buf, ch);
		}
		data++;
	}

	if (LEN(json->buf) > 0) {
		node->text = vstring_memcat(json->dbuf, node->text,
				(const char*) STR(json->buf), LEN(json->buf));
		TERM(json->dbuf, node->text);
		RESET(json->buf);
	}

	return data;
}

static const char *json_strend(JSON *json, const char *data)
{
	JSON_NODE *node = json->curr_node;
	JSON_NODE *parent;

	SKIP_SPACE(data);
	if (*data == 0) {
		return data;
	}

#define	EQ(x, y) !strcasecmp((x), ((y)))
#define	IS_NUMBER(x) (is_alldig((x)) \
		|| ((*(x) == '-' || *(x) == '+') \
			&& *((x) + 1) != 0 && is_alldig((x) + 1)))

	if (node->parent && node->parent->type == JSON_T_ARRAY) {
		if (json->quote == 0) {
			const char* txt = (const char*) STR(node->text);

			if (EQ(txt, "null")) {
				node->type = JSON_T_A_NULL | JSON_T_LEAF;
			} else if (EQ(txt, "true") || EQ(txt, "false")) {
				node->type = JSON_T_A_BOOL | JSON_T_LEAF;
			} else if (IS_NUMBER(txt)) {
				node->type = JSON_T_A_NUMBER | JSON_T_LEAF;
			} else if (is_double(txt)) {
				node->type = JSON_T_A_DOUBLE | JSON_T_LEAF;
			} else {
				node->type = JSON_T_A_STRING | JSON_T_LEAF;
			}
		} else {
			node->type = JSON_T_A_STRING | JSON_T_LEAF;
		}
	} else if (json->quote == 0) {
		const char* txt = (const char*) STR(node->text);

		if (EQ(txt, "null")) {
			node->type = JSON_T_NULL | JSON_T_LEAF;
		} else if (EQ(txt, "true") || EQ(txt, "false")) {
			node->type = JSON_T_BOOL | JSON_T_LEAF;
		} else if (IS_NUMBER(txt)) {
			node->type = JSON_T_NUMBER | JSON_T_LEAF;
		} else if (is_double(txt)) {
			node->type = JSON_T_DOUBLE | JSON_T_LEAF;
		} else {
			node->type = JSON_T_STRING | JSON_T_LEAF;
		}
	} else {
		node->type = JSON_T_STRING | JSON_T_LEAF;
	}


	if (*data == ',' || *data == ';') {
		json->status = JSON_S_NEXT;
		return data;
	}

	parent = json_node_parent(json->curr_node);
	if (*data != parent->right_ch) {  /* xxx */
		data++;
		return data;
	}

	data++;
	if (parent == json->root) {
		json->finish = 1;
		return data;
	}

	json->curr_node = parent;
	json->status    = JSON_S_NEXT;
	return data;
}

/* ���Է������ڵ����һ���ֵܽڵ㣬�������ҵ��ָ��� ',' �� ';' */

static const char *json_brother(JSON *json, const char *data)
{
	JSON_NODE *parent;

	if (json->curr_node == json->root) {
		json->finish = 1;
		return data;
	}

	SKIP_SPACE(data);
	if (*data == 0) {
		return data;
	}

	/* ���������ڵ�Ľ��������� json ����������� */
	parent = json_node_parent(json->curr_node);
	assert(parent);

	if (*data == ',' || *data == ';') {
		data++;

		if (parent->left_ch == '{') {
			json->status = JSON_S_MEMBER;
		} else if (parent->left_ch == '[') {
			json->status = JSON_S_ELEMENT;
		} else {
			json->status = JSON_S_NEXT;
		}

		json->curr_node = parent;
		return data;
	}

	if (*data == parent->right_ch) {
		data++;
		if (parent == json->root) {
			json->finish = 1;
			return data;
		}

		json->curr_node = parent;
		/* ��ѯ���ڵ����һ���ֵܽڵ� */
		json->status = JSON_S_NEXT;
		return data;
	}

	if (parent->left_ch == '{') {
		json->status = JSON_S_MEMBER;
	} else if (parent->left_ch == '[') {
		json->status = JSON_S_ELEMENT;
	} else {
		json->status = JSON_S_NEXT;
	}

	json->curr_node = parent;
	return data;
}

/* ״̬�����ݽṹ���� */

struct JSON_STATUS_MACHINE {
	/* ״̬�� */
	int   status;

	/* ״̬�������� */
	const char *(*callback) (JSON*, const char*);
};

static struct JSON_STATUS_MACHINE status_tab[] = {
	{ JSON_S_ROOT,		json_root	},	/* json root node */
	{ JSON_S_OBJ,		json_obj	},	/* json obj node */
	{ JSON_S_MEMBER,	json_member	},
	{ JSON_S_ARRAY,		json_array	},	/* json array node */
	{ JSON_S_ELEMENT,	json_element	},
	{ JSON_S_PAIR,		json_pair	},	/* json pair node */
	{ JSON_S_NEXT,		json_brother	},	/* json brother node */
	{ JSON_S_TAG,		json_tag	},	/* json tag name */
	{ JSON_S_VALUE,		json_value	},	/* json node's value */
	{ JSON_S_COLON,		json_colon	},	/* json tag's ':' */
	{ JSON_S_STRING,	json_string	},
	{ JSON_S_STREND,	json_strend	},
};

const char* json_update(JSON *json, const char *data)
{
	const char *ptr = data;

	if (data == NULL) {
		return "";
	}

	/* ����Ƿ��Ѿ�������� */
	if (json->finish) {
		return ptr;
	}

	/* json ������״̬��ѭ��������� */
	while (*ptr && !json->finish) {
		ptr = status_tab[json->status].callback(json, ptr);
	}

	return ptr;
}

int json_finish(JSON *json)
{
	return json->finish;
}
