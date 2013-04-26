
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
static char sccsid[] = "@(#)strerror.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#if defined(NLS)
#include <nl_types.h>
#endif

#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#define	UPREFIX		"Unknown error"

/*
 * Define a buffer size big enough to describe a 64-bit signed integer
 * converted to ASCII decimal (19 bytes), with an optional leading sign
 * (1 byte); finally, we get the prefix, delimiter (": ") and a trailing
 * NUL from UPREFIX.
 */
#define	EBUFSIZE	(20 + 2 + sizeof(UPREFIX))

/*
 * Doing this by hand instead of linking with stdio(3) avoids bloat for
 * statically linked binaries.
 */
static void
errstr(int num, char *uprefix, char *buf, size_t len)
{
	char *t;
	unsigned int uerr;
	char tmp[EBUFSIZE];

	t = tmp + sizeof(tmp);
	*--t = '\0';
	uerr = (num >= 0) ? num : -num;
	do {
		*--t = "0123456789"[uerr % 10];
	} while (uerr /= 10);
	if (num < 0)
		*--t = '-';
	*--t = ' ';
	*--t = ':';
	strlcpy(buf, uprefix, len);
	strlcat(buf, t, len);
}

int
strerror_r(int errnum, char *strerrbuf, size_t buflen)
{
	int retval = 0;
#if defined(NLS)
	int saved_errno = errno;
	nl_catd catd;
	catd = catopen("libc", NL_CAT_LOCALE);
#endif

	if (errnum < 0 || errnum >= sys_nerr) {
		errstr(errnum,
#if defined(NLS)
			catgets(catd, 1, 0xffff, UPREFIX),
#else
			UPREFIX,
#endif
			strerrbuf, buflen);
		retval = EINVAL;
	} else {
		if (strlcpy(strerrbuf,
#if defined(NLS)
			catgets(catd, 1, errnum, sys_errlist[errnum]),
#else
			sys_errlist[errnum],
#endif
			buflen) >= buflen)
		retval = ERANGE;
	}

#if defined(NLS)
	catclose(catd);
	errno = saved_errno;
#endif

	return (retval);
}

char *
strerror(int num)
{
	static char ebuf[NL_TEXTMAX];

	if (strerror_r(num, ebuf, sizeof(ebuf)) != 0)
		errno = EINVAL;
	return (ebuf);
}