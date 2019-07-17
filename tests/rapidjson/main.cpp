#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>
#include <thread>
#include <string>
#include "rapidjson/document.h"

static void thread_main(std::string& data)
{
	rapidjson::Document d;
	d.Parse(data.c_str());
	printf("rapid json parse ok!\r\n");
	printf("Enter any key to exit ...");
	fflush(stdout);
	getchar();
}

static bool load(const std::string& file, std::string& out)
{
	FILE* fp = fopen(file.c_str(), "r");
	if (fp == NULL) {
		printf("open %s error %s\r\n", file.c_str(), strerror(errno));
		return false;
	}

	char buf[8192];
	while (!feof(fp)) {
		size_t n = fread(buf, 1, sizeof(buf) - 1, fp);
		if (n == 0) {
			break;
		}
		buf[n] = 0;
		out += buf;
	}
	fclose(fp);
	return true;
}

static void usage(const char* procname)
{
	printf("usage: %s -h [help] -f json_file\r\n", procname);
}

int main(int argc, char* argv[])
{
	int  ch;
	std::string file("json.txt");
	std::string buf;

	while ((ch = getopt(argc, argv, "hf:")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'f':
			file = optarg;
			break;
		default:
			break;
		}
	}

	if (!load(file, buf)) {
		return 1;
	}

	std::thread thr(thread_main, ref(buf));
	thr.join();
	return 0;
}
