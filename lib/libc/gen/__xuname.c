
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
/*static char sccsid[] = "From: @(#)uname.c	8.1 (Berkeley) 1/4/94";*/
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int
__xuname(int namesize, void *namebuf)
{
	int mib[2], rval;
	size_t len;
	char *p, *q;
	int oerrno;

	rval = 0;
	q = (char *)namebuf;

	mib[0] = CTL_KERN;

	if ((p = getenv("UNAME_s")))
		strlcpy(q, p, namesize);
	else {
		mib[1] = KERN_OSTYPE;
		len = namesize;
		oerrno = errno;
		if (sysctl(mib, 2, q, &len, NULL, 0) == -1) {
			if (errno == ENOMEM)
				errno = oerrno;
			else
				rval = -1;
		}
		q[namesize - 1] = '\0';
	}
	q += namesize;

	mib[1] = KERN_HOSTNAME;
	len = namesize;
	oerrno = errno;
	if (sysctl(mib, 2, q, &len, NULL, 0) == -1) {
		if (errno == ENOMEM)
			errno = oerrno;
		else
			rval = -1;
	}
	q[namesize - 1] = '\0';
	q += namesize;

	if ((p = getenv("UNAME_r")))
		strlcpy(q, p, namesize);
	else {
		mib[1] = KERN_OSRELEASE;
		len = namesize;
		oerrno = errno;
		if (sysctl(mib, 2, q, &len, NULL, 0) == -1) {
			if (errno == ENOMEM)
				errno = oerrno;
			else
				rval = -1;
		}
		q[namesize - 1] = '\0';
	}
	q += namesize;

	if ((p = getenv("UNAME_v")))
		strlcpy(q, p, namesize);
	else {

		/*
		 * The version may have newlines in it, turn them into
		 * spaces.
		 */
		mib[1] = KERN_VERSION;
		len = namesize;
		oerrno = errno;
		if (sysctl(mib, 2, q, &len, NULL, 0) == -1) {
			if (errno == ENOMEM)
				errno = oerrno;
			else
				rval = -1;
		}
		q[namesize - 1] = '\0';
		for (p = q; len--; ++p) {
			if (*p == '\n' || *p == '\t') {
				if (len > 1)
					*p = ' ';
				else
					*p = '\0';
			}
		}
	}
	q += namesize;

	if ((p = getenv("UNAME_m")))
		strlcpy(q, p, namesize);
	else {
		mib[0] = CTL_HW;
		mib[1] = HW_MACHINE;
		len = namesize;
		oerrno = errno;
		if (sysctl(mib, 2, q, &len, NULL, 0) == -1) {
			if (errno == ENOMEM)
				errno = oerrno;
			else
				rval = -1;
		}
		q[namesize - 1] = '\0';
	}

	return (rval);
}