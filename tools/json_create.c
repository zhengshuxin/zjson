#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>

static void usage(const char *procname)
{
	printf("usage: %s -h [help]\r\n"
		"  -f to_file[default: json.txt]\r\n"
		"  -n nested number[default: 1000]\r\n", procname);
}

int main(int argc, char* argv[])
{
	FILE *fp;
	char  filename[128];
	int   ch, max = 1000, i;
	const char *s;

	snprintf(filename, sizeof(filename), "json.txt");

	while ((ch = getopt(argc, argv, "hn:f:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'n':
			max = atoi(optarg);
			break;
		case 'f':
			snprintf(filename, sizeof(filename), "%s", optarg);
			break;
		default:
			usage(argv[0]);
			return 0;
		}
	}

	fp = fopen(filename, "w");
	if  (fp == NULL) {
		printf("open %s error %s\r\n", filename, strerror(errno));
		return 1;
	}

	for (i = 0; i < max; i++) {
		s = "{\"a\":";
		if  (fwrite(s, strlen(s), 1, fp) != 1) {
			printf("fwrite to %s error %s\r\n",
				filename, strerror(errno));
			fclose(fp);
			return 1;
		}
	}

	s = "\"b\"";
	if (fwrite(s, strlen(s), 1, fp) != 1) {
		printf("fwrite to %s error %s\r\n", filename, strerror(errno));
		fclose(fp);
		return 1;
	}

	for (i = 0; i < max; i++) {
		s = "}";
		if  (fwrite(s, strlen(s), 1, fp) != 1) {
			printf("fwrite to %s error %s\r\n",
				filename, strerror(errno));
			fclose(fp);
			return 1;
		}
	}

	fclose(fp);
	return 0;
}
