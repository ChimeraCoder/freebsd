
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
static char sccsid[] = "@(#)perror.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"

void
perror(const char *s)
{
	char msgbuf[NL_TEXTMAX];
	struct iovec *v;
	struct iovec iov[4];

	v = iov;
	if (s != NULL && *s != '\0') {
		v->iov_base = (char *)s;
		v->iov_len = strlen(s);
		v++;
		v->iov_base = ": ";
		v->iov_len = 2;
		v++;
	}
	strerror_r(errno, msgbuf, sizeof(msgbuf));
	v->iov_base = msgbuf;
	v->iov_len = strlen(v->iov_base);
	v++;
	v->iov_base = "\n";
	v->iov_len = 1;
	FLOCKFILE(stderr);
	__sflush(stderr);
	(void)_writev(stderr->_file, iov, (v - iov) + 1);
	stderr->_flags &= ~__SOFF;
	FUNLOCKFILE(stderr);
}