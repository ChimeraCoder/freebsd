
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
 * pathchk -- check pathnames
 *
 * Check whether files could be created with the names specified on the
 * command line. If -p is specified, check whether the pathname is portable
 * to all POSIX systems.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/stat.h>

#include <err.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int	 check(const char *);
static int	 portable(const char *);
static void	 usage(void);

static int	 pflag;			/* Perform portability checks */
static int	 Pflag;			/* Check for empty paths, leading '-' */

int
main(int argc, char *argv[])
{
	int ch, rval;
	const char *arg;

	while ((ch = getopt(argc, argv, "pP")) > 0) {
		switch (ch) {
		case 'p':
			pflag = 1;
			break;
		case 'P':
			Pflag = 1;
			break;
		default:
			usage();
			/*NOTREACHED*/
		}
	}
	argc -= optind;
	argv += optind;

	if (argc == 0)
		usage();

	rval = 0;
	while ((arg = *argv++) != NULL)
		rval |= check(arg);

	exit(rval);
}

static void
usage(void)
{

	fprintf(stderr, "usage: pathchk [-p] pathname ...\n");
	exit(1);
}

static int
check(const char *path)
{
	struct stat sb;
	long complen, namemax, pathmax, svnamemax;
	int badch, last;
	char *end, *p, *pathd;

	if ((pathd = strdup(path)) == NULL)
		err(1, "strdup");

	p = pathd;

	if (Pflag && *p == '\0') {
		warnx("%s: empty pathname", path);
		goto bad;
	}
	if ((Pflag || pflag) && (*p == '-' || strstr(p, "/-") != NULL)) {
		warnx("%s: contains a component starting with '-'", path);
		goto bad;
	}

	if (!pflag) {
		errno = 0;
		namemax = pathconf(*p == '/' ? "/" : ".", _PC_NAME_MAX);
		if (namemax == -1 && errno != 0)
			namemax = NAME_MAX;
	} else
		namemax = _POSIX_NAME_MAX;

	for (;;) {
		p += strspn(p, "/");
		complen = (long)strcspn(p, "/");
		end = p + complen;
		last = *end == '\0';
		*end = '\0';

		if (namemax != -1 && complen > namemax) {
			warnx("%s: %s: component too long (limit %ld)", path,
			    p, namemax);
			goto bad;
		}

		if (!pflag && stat(pathd, &sb) == -1 && errno != ENOENT) {
			warn("%s: %.*s", path, (int)(strlen(pathd) -
			    complen - 1), pathd);
			goto bad;
		}

		if (pflag && (badch = portable(p)) >= 0) {
			warnx("%s: %s: component contains non-portable "
			    "character `%c'", path, p, badch);
			goto bad;
		}

		if (last)
			break;

		if (!pflag) {
			errno = 0;
			svnamemax = namemax;
			namemax = pathconf(pathd, _PC_NAME_MAX);
			if (namemax == -1 && errno != 0)
				namemax = svnamemax;
		}

		*end = '/';
		p = end + 1;
	}

	if (!pflag) {
		errno = 0;
		pathmax = pathconf(path, _PC_PATH_MAX);
		if (pathmax == -1 && errno != 0)
			pathmax = PATH_MAX;
	} else
		pathmax = _POSIX_PATH_MAX;
	if (pathmax != -1 && strlen(path) >= (size_t)pathmax) {
		warnx("%s: path too long (limit %ld)", path, pathmax - 1);
		goto bad;
	}

	free(pathd);
	return (0);

bad:	free(pathd);
	return (1);
}

/*
 * Check whether a path component contains only portable characters. Return
 * the first non-portable character found.
 */
static int
portable(const char *path)
{
	static const char charset[] =
	    "abcdefghijklmnopqrstuvwxyz"
	    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	    "0123456789._-";
	long s;

	s = strspn(path, charset);
	if (path[s] != '\0')
		return (path[s]);

	return (-1);
}