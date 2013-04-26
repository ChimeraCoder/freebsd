
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
 * This program was originally written long ago, originally for a non
 * BSD-like OS without mkstemp().  It's been modified over the years
 * to use mkstemp() rather than the original O_CREAT|O_EXCL/fstat/lstat
 * etc style hacks.
 * A cleanup, misc options and mkdtemp() calls were added to try and work
 * more like the OpenBSD version - which was first to publish the interface.
 */

#include <err.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef lint
static const char rcsid[] =
	"$FreeBSD$";
#endif /* not lint */

static void usage(void);

int
main(int argc, char **argv)
{
	int c, fd, ret;
	char *tmpdir;
	const char *prefix;
	char *name;
	int dflag, qflag, tflag, uflag;

	ret = dflag = qflag = tflag = uflag = 0;
	prefix = "mktemp";
	name = NULL;

	while ((c = getopt(argc, argv, "dqt:u")) != -1)
		switch (c) {
		case 'd':
			dflag++;
			break;

		case 'q':
			qflag++;
			break;

		case 't':
			prefix = optarg;
			tflag++;
			break;

		case 'u':
			uflag++;
			break;

		default:
			usage();
		}

	argc -= optind;
	argv += optind;

	if (!tflag && argc < 1) {
		tflag = 1;
		prefix = "tmp";
	}

	if (tflag) {
		tmpdir = getenv("TMPDIR");
		if (tmpdir == NULL)
			asprintf(&name, "%s%s.XXXXXXXX", _PATH_TMP, prefix);
		else
			asprintf(&name, "%s/%s.XXXXXXXX", tmpdir, prefix);
		/* if this fails, the program is in big trouble already */
		if (name == NULL) {
			if (qflag)
				return (1);
			else
				errx(1, "cannot generate template");
		}
	}
		
	/* generate all requested files */
	while (name != NULL || argc > 0) {
		if (name == NULL) {
			name = strdup(argv[0]);
			argv++;
			argc--;
		}

		if (dflag) {
			if (mkdtemp(name) == NULL) {
				ret = 1;
				if (!qflag)
					warn("mkdtemp failed on %s", name);
			} else {
				printf("%s\n", name);
				if (uflag)
					rmdir(name);
			}
		} else {
			fd = mkstemp(name);
			if (fd < 0) {
				ret = 1;
				if (!qflag)
					warn("mkstemp failed on %s", name);
			} else {
				close(fd);
				if (uflag)
					unlink(name);
				printf("%s\n", name);
			}
		}
		if (name)
			free(name);
		name = NULL;
	}
	return (ret);
}

static void
usage(void)
{
	fprintf(stderr,
		"usage: mktemp [-d] [-q] [-t prefix] [-u] template ...\n");
	fprintf(stderr,
		"       mktemp [-d] [-q] [-u] -t prefix \n");
	exit (1);
}