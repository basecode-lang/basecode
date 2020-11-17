/*
 Author: José Bollo <jobol@nonadev.net>
 Author: José Bollo <jose.bollo@iot.bzh>

 https://gitlab.com/jobol/mustach

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
*/

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>

#include <json-c/json.h>

#include "../mustach-json-c.h"

static const size_t BLOCKSIZE = 8192;

static char *readfile(const char *filename)
{
	int f;
	struct stat s;
	char *result;
	size_t size, pos;
	ssize_t rc;

	result = NULL;
	if (filename[0] == '-' &&  filename[1] == 0)
		f = dup(0);
	else
		f = open(filename, O_RDONLY);
	if (f < 0) {
		fprintf(stderr, "Can't open file: %s\n", filename);
		exit(1);
	}

	fstat(f, &s);
	switch (s.st_mode & S_IFMT) {
	case S_IFREG:
		size = s.st_size;
		break;
	case S_IFSOCK:
	case S_IFIFO:
		size = BLOCKSIZE;
		break;
	default:
		fprintf(stderr, "Bad file: %s\n", filename);
		exit(1);
	}

	pos = 0;
	result = malloc(size + 1);
	do {
		if (result == NULL) {
			fprintf(stderr, "Out of memory\n");
			exit(1);
		}
		rc = read(f, &result[pos], (size - pos) + 1);
		if (rc < 0) {
			fprintf(stderr, "Error while reading %s\n", filename);
			exit(1);
		}
		if (rc > 0) {
			pos += (size_t)rc;
			if (pos > size) {
				size = pos + BLOCKSIZE;
				result = realloc(result, size + 1);
			}
		}
	} while(rc > 0);

	close(f);
	result[pos] = 0;
	return result;
}

enum { None, Upper, Lower } mode = None;

int uwrite(void *closure, const char *buffer, size_t size)
{
	switch(mode) {
	case None:
		fwrite(buffer, size, 1, stdout);
		break;
	case Upper:
		while(size--)
			fputc(toupper(*buffer++), stdout);
		break;
	case Lower:
		while(size--)
			fputc(tolower(*buffer++), stdout);
		break;
	}
	return 0;
}

int main(int ac, char **av)
{
	struct json_object *o;
	char *t;
	char *prog = *av;
	int s;

	if (*++av) {
		o = json_object_from_file(av[0]);
		if (o == NULL) {
			fprintf(stderr, "Aborted: null json (file %s)\n", av[0]);
			exit(1);
		}
		while(*++av) {
			if (!strcmp(*av, "-U"))
				mode = Upper;
			else if  (!strcmp(*av, "-l"))
				mode = Lower;
			else if  (!strcmp(*av, "-x"))
				mode = None;
			else {
				t = readfile(*av);
				s = umustach_json_c(t, o, uwrite, NULL);
				if (s != 0)
					fprintf(stderr, "Template error %d\n", s);
				free(t);
			}
		}
		json_object_put(o);
	}
	return 0;
}

