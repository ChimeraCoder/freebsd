
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

#if 0
#ifndef lint
static char sccsid[] = "@(#)getmntopts.c	8.3 (Berkeley) 3/29/95";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/uio.h>

#include <err.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mntopts.h"

int getmnt_silent = 0;

void
getmntopts(const char *options, const struct mntopt *m0, int *flagp,
	int *altflagp)
{
	const struct mntopt *m;
	int negative, len;
	char *opt, *optbuf, *p;
	int *thisflagp;

	/* Copy option string, since it is about to be torn asunder... */
	if ((optbuf = strdup(options)) == NULL)
		err(1, NULL);

	for (opt = optbuf; (opt = strtok(opt, ",")) != NULL; opt = NULL) {
		/* Check for "no" prefix. */
		if (opt[0] == 'n' && opt[1] == 'o') {
			negative = 1;
			opt += 2;
		} else
			negative = 0;

		/*
		 * for options with assignments in them (ie. quotas)
		 * ignore the assignment as it's handled elsewhere
		 */
		p = strchr(opt, '=');
		if (p != NULL)
			 *++p = '\0';

		/* Scan option table. */
		for (m = m0; m->m_option != NULL; ++m) {
			len = strlen(m->m_option);
			if (strncasecmp(opt, m->m_option, len) == 0)
				if (opt[len] == '\0' || opt[len] == '=')
					break;
		}

		/* Save flag, or fail if option is not recognized. */
		if (m->m_option) {
			thisflagp = m->m_altloc ? altflagp : flagp;
			if (negative == m->m_inverse)
				*thisflagp |= m->m_flag;
			else
				*thisflagp &= ~m->m_flag;
		} else if (!getmnt_silent) {
			errx(1, "-o %s: option not supported", opt);
		}
	}

	free(optbuf);
}

void
rmslashes(char *rrpin, char *rrpout)
{
	char *rrpoutstart;

	*rrpout = *rrpin;
	for (rrpoutstart = rrpout; *rrpin != '\0'; *rrpout++ = *rrpin++) {

		/* skip all double slashes */
		while (*rrpin == '/' && *(rrpin + 1) == '/')
			 rrpin++;
	}

	/* remove trailing slash if necessary */
	if (rrpout - rrpoutstart > 1 && *(rrpout - 1) == '/')
		*(rrpout - 1) = '\0';
	else
		*rrpout = '\0';
}

int
checkpath(const char *path, char *resolved)
{
	struct stat sb;

	if (realpath(path, resolved) == NULL || stat(resolved, &sb) != 0)
		return (1);
	if (!S_ISDIR(sb.st_mode)) {
		errno = ENOTDIR;
		return (1);
	}
	return (0);
}

void
build_iovec(struct iovec **iov, int *iovlen, const char *name, void *val,
	    size_t len)
{
	int i;

	if (*iovlen < 0)
		return;
	i = *iovlen;
	*iov = realloc(*iov, sizeof **iov * (i + 2));
	if (*iov == NULL) {
		*iovlen = -1;
		return;
	}
	(*iov)[i].iov_base = strdup(name);
	(*iov)[i].iov_len = strlen(name) + 1;
	i++;
	(*iov)[i].iov_base = val;
	if (len == (size_t)-1) {
		if (val != NULL)
			len = strlen(val) + 1;
		else
			len = 0;
	}
	(*iov)[i].iov_len = (int)len;
	*iovlen = ++i;
}

/*
 * This function is needed for compatibility with parameters
 * which used to use the mount_argf() command for the old mount() syscall.
 */
void
build_iovec_argf(struct iovec **iov, int *iovlen, const char *name,
    const char *fmt, ...)
{
	va_list ap;
	char val[255] = { 0 };

	va_start(ap, fmt);
	vsnprintf(val, sizeof(val), fmt, ap);
	va_end(ap);
	build_iovec(iov, iovlen, name, strdup(val), (size_t)-1);
}