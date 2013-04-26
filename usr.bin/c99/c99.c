
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

/*
 * c99 -- compile standard C programs
 *
 * This is essentially a wrapper around the system C compiler that forces
 * the compiler into C99 mode and handles some of the standard libraries
 * specially.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char **args;
static u_int cargs, nargs;

static void addarg(const char *);
static void addlib(const char *);
static void usage(void);

int
main(int argc, char *argv[])
{
	int ch, i;

	args = NULL;
	cargs = nargs = 0;

	while ((ch = getopt(argc, argv, "cD:EgI:L:o:O:sU:l:")) != -1) {
		if (ch == 'l') {
			/* Gone too far. Back up and get out. */
			if (argv[optind - 1][0] == '-')
				optind -= 1;
			else
				optind -= 2;
			break;
		} else if (ch == '?')
			usage();
	}

	addarg("/usr/bin/cc");
	addarg("-std=iso9899:1999");
	addarg("-pedantic");
	for (i = 1; i < optind; i++)
		addarg(argv[i]);
	while (i < argc) {
		if (strncmp(argv[i], "-l", 2) == 0) {
			if (argv[i][2] != '\0')
				addlib(argv[i++] + 2);
			else {
				if (argv[++i] == NULL)
					usage();
				addlib(argv[i++]);
			}
		} else
			addarg(argv[i++]);
	}
	execv("/usr/bin/cc", args);
	err(1, "/usr/bin/cc");
}

static void
addarg(const char *item)
{
	if (nargs + 1 >= cargs) {
		cargs += 16;
		if ((args = realloc(args, sizeof(*args) * cargs)) == NULL)
			err(1, "malloc");
	}
	if ((args[nargs++] = strdup(item)) == NULL)
		err(1, "strdup");
	args[nargs] = NULL;
}

static void
addlib(const char *lib)
{

	if (strcmp(lib, "pthread") == 0)
		/* FreeBSD's gcc uses -pthread instead of -lpthread. */
		addarg("-pthread");
	else if (strcmp(lib, "rt") == 0)
		/* librt functionality is in libc or unimplemented. */
		;
	else if (strcmp(lib, "xnet") == 0)
		/* xnet functionality is in libc. */
		;
	else {
		addarg("-l");
		addarg(lib);
	}
}

static void
usage(void)
{
	(void)fprintf(stderr, "%s\n%s\n",
"usage: c99 [-cEgs] [-D name[=value]] ... [-I directory] ... [-L directory] ...",
"       [-o outfile] [-O optlevel] [-U name] ... operand ...");
	exit(1);
}