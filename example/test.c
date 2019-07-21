#include "StdAfx.h"
#include "json/vstring.h"
#include "json/array.h"
#include "json/json.h"

#if 1
static const char* json_data = \
    "{ 'menu name': {\r\n"
    "    'id:file': 'file',\r\n"
    "    'value{': 'File',\r\n"
    "    'popup{}': {\r\n"
    "        'menuitem1}': [\r\n"
    "            {'value': 'New1', 'onclick': 'CreateNewDoc()'},\r\n"
    "            {'value': 'Open1', 'onclick': 'OpenDoc()'},\r\n"
    "            {'value': 'Close1', 'onclick': 'CloseDoc()'}\r\n"
    "        ],\r\n"
    "        'menuname[]': 'hello world',\r\n"
    "        'inner': { 'value' : 'new ', 'value' : 'open' },\r\n"
    "        'menuitem2': [\r\n"
    "            {'value': 'New2', 'onclick': 'CreateNewDoc()'},\r\n"
    "            {'value': 'Open2', 'onclick': 'OpenDoc()'},\r\n"
    "            {'value': 'Close2', 'onclick': 'CloseDoc()'},\r\n"
    "            {{'value': 'Help2', 'onclick': 'Help()'}}"
    "        ]\r\n"
    "    }\r\n"
    " }\r\n,"
    " 'help': 'hello world!',\r\n"
    " 'bool': ture,\r\n"
    " 'double-number': 3.14,\r\n"
    " 'inter-number': 3000,\r\n"
    " 'null': null,\r\n"
    " 'menuitem2': [\r\n"
    "   {'value': 'New3', 'onclick': 'CreateNewDoc()'},\r\n"
    "   {'value': 'Open3', 'onclick': 'OpenDoc()'},\r\n"
    "   {'value': 'Close3', 'onclick': 'CloseDoc()'},\r\n"
    "   [{'value': 'Save3, 'onclick': 'SaveDoc()'}]"
    " ]\r\n"
    "}\r\n"
    "{ 'hello world' }\r\n";
#else
static const char* json_data = \
    "{ 'name': [ {'name1': 'false', 'name3': true }, {'name2': 'value2'}] }";
#endif

static void dump_data(const char *filepath, const char *data, size_t len)
{
	FILE *fp = fopen(filepath, "w");
	if (fp == NULL) {
		printf("fopen %s error %s\r\n", filepath, strerror(errno));
		return;
	}

	if (fwrite(data, len, 1, fp) != 1) {
		printf("fwrite to %s error %s\r\n", filepath, strerror(errno));
	}
	fclose(fp);
}

static void show_json(JSON *json)
{
	VSTRING *buf = json_build(json);
	const char *filepath = "dump.txt";
	ARRAY *a;
	ITER it;
	RING_ITER rit;

	if (!buf) {
		printf("json_build failed!\r\n");
		return;
	}


	printf("--------------------------------------------\r\n");
	foreach(it, json) {
		JSON_NODE *node = it.data;
		printf("tag=%s, type=%s, %d\r\n", node->ltag ?
			VSTRING_STR(node->ltag) : "none",
			json_node_type(node->type), node->type);
	}

	printf("--------------------------------------------\r\n");
	printf("%s\r\n", VSTRING_STR(buf));

	printf("--------------------------------------------\r\n");
	ring_foreach(rit, &json->root->children) {
		JSON_NODE *node = ring_to_appl(rit.ptr, JSON_NODE, node);
		printf("tag: %s, type: %s\r\n", VSTRING_STR(node->ltag),
			json_node_type(node->type));
	}

	printf("--------------------------------------------\r\n");
	a = json_getElementsByTags(json, "menuitem2/value");
	if (a) {
		foreach(it, a) {
			JSON_NODE *node = (JSON_NODE*) it.data;
			printf("tag: %s, type: %s, value=%s\r\n",
				VSTRING_STR(node->ltag),
				json_node_type(node->type),
				VSTRING_STR(node->text));
		}

		json_free_array(a);
	}

	dump_data(filepath, VSTRING_STR(buf), VSTRING_LEN(buf));
}

static void test(void)
{
	JSON *json = json_alloc();
	json_update(json, json_data);


	if (json_finish(json)) {
		printf("json parsing ok!\r\n");
	} else {
		printf("json parsing error!\r\n");
	}

	printf("Enter any key to call show_json ...");
	fflush(stdout);
	getchar();

	show_json(json);

	printf("Enter any key to call json_free ...");
	fflush(stdout);

	getchar();
	json_free(json);
}

int main(void)
{
	printf("JSON_NODE's size is %ld\r\n", sizeof(JSON_NODE));
	test();
	return 0;
}
