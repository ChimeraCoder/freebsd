
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

#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "ofw_options.h"

static int	action(char *);
static void	dump_config(void);
static void	usage(void);

static void
usage(void)
{

	fprintf(stderr,
	    "usage: eeprom -a\n"
	    "       eeprom [-] name[=value] ...\n");
	exit(EX_USAGE);
}

int
main(int argc, char *argv[])
{
	int do_stdin, opt;
	int aflag, rv;
	char *cp;
	char line[BUFSIZ];

	aflag = do_stdin = 0;
	rv = EX_OK;
	while ((opt = getopt(argc, argv, "-a")) != -1) {
		switch (opt) {
		case '-':
			if (aflag)
				usage();
			do_stdin = 1;
			break;
		case 'a':
			if (do_stdin)
				usage();
			aflag = 1;
			break;
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (aflag) {
		if (argc != 0)
			usage();
		dump_config();
	} else {
		if (do_stdin) {
			while (fgets(line, BUFSIZ, stdin) != NULL &&
			    rv == EX_OK) {
				if (line[0] == '\n')
					continue;
				if ((cp = strrchr(line, '\n')) != NULL)
					*cp = '\0';
				rv = action(line);
			}
			if (ferror(stdin))
				err(EX_NOINPUT, "stdin");
		} else {
			if (argc == 0)
				usage();
			while (argc && rv == EX_OK) {
				rv = action(*argv);
				++argv;
				--argc;
			}
		}
	}
	return (rv);
}

static int
action(char *line)
{
	int rv;
	char *keyword, *arg;

	keyword = strdup(line);
	if (keyword == NULL)
		err(EX_OSERR, "malloc() failed");
	if ((arg = strrchr(keyword, '=')) != NULL)
		*arg++ = '\0';
	switch (rv = ofwo_action(keyword, arg)) {
		case EX_UNAVAILABLE:
			warnx("nothing available for '%s'.", keyword);
			break;
		case EX_DATAERR:
			warnx("invalid value '%s' for '%s'.", arg, keyword);
			break;
	}
	free(keyword);
	return(rv);
}

static void
dump_config(void)
{

	ofwo_dump();
}