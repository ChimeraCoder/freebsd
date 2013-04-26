
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <err.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "t.h"

const char *t_progname;

static int verbose;

void
t_verbose(const char *fmt, ...)
{
	va_list ap;

	if (verbose) {
		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}
}

static void
usage(void)
{

	fprintf(stderr, "usage: [-v] %s\n", t_progname);
	exit(1);
}

int
main(int argc, char *argv[])
{
	const struct t_test **t_plan;
	const char *desc;
	int n, pass, fail;
	int opt;

	if ((t_progname = strrchr(argv[0], '/')) != NULL)
		t_progname++; /* one past the slash */
	else
		t_progname = argv[0];

	while ((opt = getopt(argc, argv, "v")) != -1)
		switch (opt) {
		case 'v':
			verbose = 1;
			break;
		default:
			usage();
		}

	argc -= optind;
	argv += optind;

	/* prepare the test plan */
	if ((t_plan = t_prepare(argc, argv)) == NULL)
		errx(1, "no plan\n");

	/* count the tests */
	for (n = 0; t_plan[n] != NULL; ++n)
		/* nothing */;
	printf("1..%d\n", n);

	/* run the tests */
	for (n = pass = fail = 0; t_plan[n] != NULL; ++n) {
		desc = t_plan[n]->desc ? t_plan[n]->desc : "no description";
		if ((*t_plan[n]->func)()) {
			printf("ok %d - %s\n", n + 1, desc);
			++pass;
		} else {
			printf("not ok %d - %s\n", n + 1, desc);
			++fail;
		}
	}

	/* clean up and exit */
	t_cleanup();
	exit(fail > 0 ? 1 : 0);
}