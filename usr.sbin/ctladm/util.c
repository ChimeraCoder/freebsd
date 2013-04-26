
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
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/stdint.h>
#include <sys/types.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <camlib.h>
#include "ctladm.h"

static int verbose;

/* iget: Integer argument callback
 */
int
iget(void *hook, char *name)
{
	struct get_hook *h = (struct get_hook *)hook;
	int arg;

	if (h->got >= h->argc)
	{
		fprintf(stderr, "Expecting an integer argument.\n");
		usage(0);
		exit(1);
	}
	arg = strtol(h->argv[h->got], 0, 0);
	h->got++;

	if (verbose && name && *name)
		printf("%s: %d\n", name, arg);

	return arg;
}

/* cget: char * argument callback
 */
char *
cget(void *hook, char *name)
{
	struct get_hook *h = (struct get_hook *)hook;
	char *arg;

	if (h->got >= h->argc)
	{
		fprintf(stderr, "Expecting a character pointer argument.\n");
		usage(0);
		exit(1);
	}
	arg = h->argv[h->got];
	h->got++;

	if (verbose && name)
		printf("cget: %s: %s", name, arg);

	return arg;
}

/* arg_put: "put argument" callback
 */
void
arg_put(void *hook __unused, int letter, void *arg, int count, char *name)
{
	if (verbose && name && *name)
		printf("%s:  ", name);

	switch(letter)
	{
		case 'i':
		case 'b':
		printf("%jd ", (intmax_t)(intptr_t)arg);
		break;

		case 'c':
		case 'z':
		{
			char *p;

			p = malloc(count + 1);
			if (p == NULL) {
				fprintf(stderr, "can't malloc memory for p\n");
				exit(1);
			}

			bzero(p, count +1);
			strncpy(p, (char *)arg, count);
			if (letter == 'z')
			{
				int i;
				for (i = count - 1; i >= 0; i--)
					if (p[i] == ' ')
						p[i] = 0;
					else
						break;
			}
			printf("%s ", p);

			free(p);
		}

		break;

		default:
		printf("Unknown format letter: '%c'\n", letter);
	}
	if (verbose)
		putchar('\n');
}