#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

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

static double stamp_sub(const struct timeval* from, const struct timeval* sub)
{
	struct timeval res;

	memcpy(&res, from, sizeof(struct timeval));

	res.tv_usec -= sub->tv_usec;
	if (res.tv_usec < 0) {
		--res.tv_sec;
		res.tv_usec += 1000000;
	}

	res.tv_sec -= sub->tv_sec;
	return res.tv_sec * 1000.0 + res.tv_usec / 1000.0;
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
	struct timeval begin, end;
	double spent;

	if (buf == NULL) {
		return;
	}

	json = json_alloc();

	gettimeofday(&begin, NULL);

	json_update(json, buf);
	if (json_finish(json)) {
		printf("json parsing ok!\r\n");
	} else {
		printf("json parsing error!\r\n");
	}

	gettimeofday(&end, NULL);
	spent = stamp_sub(&end, &begin);

	printf("spent=%.2f, file size=%ld\r\n", spent, size);

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

static void usage(const char *procname)
{
	printf("usage: %s -h [help] -f json_file_path\r\n", procname);
}

int main(int argc, char *argv[])
{
	int ch;
	char file[128];

	file[0] = 0;

	while ((ch = getopt(argc, argv, "hf:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'f':
			snprintf(file, sizeof(file), "%s", optarg);
			break;
		default:
			break;
		}
	}

	printf("JSON_NODE's size is %ld\r\n", sizeof(JSON_NODE));

	if (file[0] == 0) {
		usage(argv[0]);
		return 1;
	}

	test(file);
	return 0;
}
