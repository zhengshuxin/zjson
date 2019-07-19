#include "StdAfx.h"
#include "json/vstring.h"
#include "json/json.h"

static char *load_data(const char *filepath, size_t *size)
{
	struct stat sbuf;
	FILE  *fp;
	char  *buf;
	size_t ret;

	if (stat(filepath, &sbuf) == -1) {
		printf("stat %s error %s\r\n", filepath, strerror(errno));
		return NULL;
	}
	if (sbuf.st_size <= 0) {
		printf("invalid file size=%ld\r\n", sbuf.st_size);
		return NULL;
	}

	fp = fopen(filepath, "r");
	if (fp == NULL) {
		printf("open %s error %s\r\n", filepath, strerror(errno));
		return NULL;
	}

	buf = (char *) malloc((size_t) sbuf.st_size + 1);
	ret = fread(buf, (size_t) sbuf.st_size, 1, fp);
	if (ret != 1) {
		printf("read from %s error %s, ret=%lu\r\n",
			filepath, strerror(errno), ret);
		free(buf);
		fclose(fp);
		return NULL;
	}

	fclose(fp);
	*size = (size_t) sbuf.st_size;
	printf("read ok, size=%ld, enter any key to continue ...", sbuf.st_size);
	fflush(stdout);
	getchar();

	return buf;
}

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

	if (!buf) {
		printf("json_build failed!\r\n");
		return;
	}

	if (VSTRING_LEN(buf) < 102400) {
		printf("%s\r\n", VSTRING_STR(buf));
	} else {
		printf("json too large %d\r\n", VSTRING_LEN(buf));
	}

	dump_data(filepath, VSTRING_STR(buf), VSTRING_LEN(buf));
}

static void test(const char *filepath)
{
	size_t size;
	char *buf = load_data(filepath, &size);
	JSON *json;

	if (buf == NULL) {
		return;
	}

	json = json_alloc();

	json_update(json, buf);
	if (json_finish(json)) {
		printf("json parsing ok!\r\n");
	} else {
		printf("json parsing error!\r\n");
	}

	free(buf);

	printf("Enter any key to call show_json ...");
	fflush(stdout);
	getchar();

	show_json(json);

	printf("Enter any key to call json_free ...");
	fflush(stdout);

	getchar();
	json_free(json);
}

int main(int argc, char *argv[])
{
#if defined(_WIN32) || defined(_WIN64)
	char file[128];

	if (argc >= 2) {
		snprintf(file, sizeof(file), "%s", argv[1]);
	}
#else
	char file[128];
	int  ch;

	snprintf(file, sizeof(file), "json.txt");

	while ((ch = getopt(argc, argv, "hf:")) > 0) {
		switch (ch) {
		case 'h':
			printf("usage: %s -h [help] -f json_file\r\n", argv[0]);
			return 0;
		case 'f':
			snprintf(file, sizeof(file), "%s", optarg);
			break;
		default:
			break;
		}
	}
#endif

	printf("JSON_NODE's size is %ld\r\n", sizeof(JSON_NODE));
	test(file);
	return 0;
}
