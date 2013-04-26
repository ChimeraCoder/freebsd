
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)mktemp.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "un-namespace.h"

char *_mktemp(char *);

static int _gettemp(char *, int *, int, int);

static const unsigned char padchar[] =
"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

int
mkstemps(char *path, int slen)
{
	int fd;

	return (_gettemp(path, &fd, 0, slen) ? fd : -1);
}

int
mkstemp(char *path)
{
	int fd;

	return (_gettemp(path, &fd, 0, 0) ? fd : -1);
}

char *
mkdtemp(char *path)
{
	return (_gettemp(path, (int *)NULL, 1, 0) ? path : (char *)NULL);
}

char *
_mktemp(char *path)
{
	return (_gettemp(path, (int *)NULL, 0, 0) ? path : (char *)NULL);
}

__warn_references(mktemp,
    "warning: mktemp() possibly used unsafely; consider using mkstemp()");

char *
mktemp(char *path)
{
	return (_mktemp(path));
}

static int
_gettemp(char *path, int *doopen, int domkdir, int slen)
{
	char *start, *trv, *suffp, *carryp;
	char *pad;
	struct stat sbuf;
	int rval;
	uint32_t rand;
	char carrybuf[MAXPATHLEN];

	if ((doopen != NULL && domkdir) || slen < 0) {
		errno = EINVAL;
		return (0);
	}

	for (trv = path; *trv != '\0'; ++trv)
		;
	if (trv - path >= MAXPATHLEN) {
		errno = ENAMETOOLONG;
		return (0);
	}
	trv -= slen;
	suffp = trv;
	--trv;
	if (trv < path || NULL != strchr(suffp, '/')) {
		errno = EINVAL;
		return (0);
	}

	/* Fill space with random characters */
	while (trv >= path && *trv == 'X') {
		rand = arc4random_uniform(sizeof(padchar) - 1);
		*trv-- = padchar[rand];
	}
	start = trv + 1;

	/* save first combination of random characters */
	memcpy(carrybuf, start, suffp - start);

	/*
	 * check the target directory.
	 */
	if (doopen != NULL || domkdir) {
		for (; trv > path; --trv) {
			if (*trv == '/') {
				*trv = '\0';
				rval = stat(path, &sbuf);
				*trv = '/';
				if (rval != 0)
					return (0);
				if (!S_ISDIR(sbuf.st_mode)) {
					errno = ENOTDIR;
					return (0);
				}
				break;
			}
		}
	}

	for (;;) {
		if (doopen) {
			if ((*doopen =
			    _open(path, O_CREAT|O_EXCL|O_RDWR, 0600)) >= 0)
				return (1);
			if (errno != EEXIST)
				return (0);
		} else if (domkdir) {
			if (mkdir(path, 0700) == 0)
				return (1);
			if (errno != EEXIST)
				return (0);
		} else if (lstat(path, &sbuf))
			return (errno == ENOENT);

		/* If we have a collision, cycle through the space of filenames */
		for (trv = start, carryp = carrybuf;;) {
			/* have we tried all possible permutations? */
			if (trv == suffp)
				return (0); /* yes - exit with EEXIST */
			pad = strchr(padchar, *trv);
			if (pad == NULL) {
				/* this should never happen */
				errno = EIO;
				return (0);
			}
			/* increment character */
			*trv = (*++pad == '\0') ? padchar[0] : *pad;
			/* carry to next position? */
			if (*trv == *carryp) {
				/* increment position and loop */
				++trv;
				++carryp;
			} else {
				/* try with new name */
				break;
			}
		}
	}
	/*NOTREACHED*/
}