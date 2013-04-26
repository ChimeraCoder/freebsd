
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "magic.h"

static void *
xrealloc(void *p, size_t n)
{
	p = realloc(p, n);
	if (p == NULL) {
		(void)fprintf(stderr, "ERROR slurping file: out of memory\n");
		exit(10);
	}
	return p;
}

static char *
slurp(FILE *fp, size_t *final_len)
{
	size_t len = 256;
	int c;
	char *l = (char *)xrealloc(NULL, len), *s = l;

	for (c = getc(fp); c != EOF; c = getc(fp)) {
		if (s == l + len) {
			l = (char *)xrealloc(l, len * 2);
			len *= 2;
		}
		*s++ = c;
	}
	if (s == l + len)
		l = (char *)xrealloc(l, len + 1);
	*s++ = '\0';

	*final_len = s - l;
	l = (char *)xrealloc(l, s - l);
	return l;
}

int
main(int argc, char **argv)
{
	struct magic_set *ms;
	const char *result;
	char *desired;
	size_t desired_len;
	int i;
	FILE *fp;

	ms = magic_open(MAGIC_NONE);
	if (ms == NULL) {
		(void)fprintf(stderr, "ERROR opening MAGIC_NONE: out of memory\n");
		return 10;
	}
	if (magic_load(ms, NULL) == -1) {
		(void)fprintf(stderr, "ERROR loading with NULL file: %s\n", magic_error(ms));
		return 11;
	}

	if (argc > 1) {
		if (argc != 3) {
			(void)fprintf(stderr, "Usage: test TEST-FILE RESULT\n");
		} else {
			if ((result = magic_file(ms, argv[1])) == NULL) {
				(void)fprintf(stderr, "ERROR loading file %s: %s\n", argv[1], magic_error(ms));
				return 12;
			} else {
				fp = fopen(argv[2], "r");
				if (fp == NULL) {
					(void)fprintf(stderr, "ERROR opening `%s': ", argv[2]);
					perror(NULL);
					return 13;
				}
				desired = slurp(fp, &desired_len);
				fclose(fp);
				(void)printf("%s: %s\n", argv[1], result);
                                if (strcmp(result, desired) != 0) {
					(void)fprintf(stderr, "Error: result was\n%s\nexpected:\n%s\n", result, desired);
					return 1;
                                }
			}
		}
	}

	magic_close(ms);
	return 0;
}