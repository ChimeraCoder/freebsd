
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
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Portions Copyright (c) 1996-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static const char sccsid[] = "@(#)herror.c	8.1 (Berkeley) 6/4/93";
static const char rcsid[] = "$Id: herror.c,v 1.3.18.1 2005/04/27 05:01:09 sra Exp $";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "port_before.h"

#include "namespace.h"
#include <sys/types.h>
#include <sys/param.h>
#include <sys/uio.h>

#include <netinet/in.h>
#include <arpa/nameser.h>

#include <netdb.h>
#include <resolv.h>
#include <string.h>
#include <unistd.h>
#include "un-namespace.h"

#include "port_after.h"

const char *h_errlist[] = {
	"Resolver Error 0 (no error)",
	"Unknown host",				/*%< 1 HOST_NOT_FOUND */
	"Host name lookup failure",		/*%< 2 TRY_AGAIN */
	"Unknown server error",			/*%< 3 NO_RECOVERY */
	"No address associated with name",	/*%< 4 NO_ADDRESS */
};
const int h_nerr = { sizeof h_errlist / sizeof h_errlist[0] };

#undef	h_errno
int	h_errno;

/*%
 * herror --
 *	print the error indicated by the h_errno value.
 */
void
herror(const char *s) {
	struct iovec iov[4], *v = iov;
	char *t;

	if (s != NULL && *s != '\0') {
		DE_CONST(s, t);
		v->iov_base = t;
		v->iov_len = strlen(t);
		v++;
		DE_CONST(": ", t);
		v->iov_base = t;
		v->iov_len = 2;
		v++;
	}
	DE_CONST(hstrerror(*__h_errno()), t);
	v->iov_base = t;
	v->iov_len = strlen(v->iov_base);
	v++;
	DE_CONST("\n", t);
	v->iov_base = t;
	v->iov_len = 1;
	_writev(STDERR_FILENO, iov, (v - iov) + 1);
}

/*%
 * hstrerror --
 *	return the string associated with a given "host" errno value.
 */
const char *
hstrerror(int err) {
	if (err < 0)
		return ("Resolver internal error");
	else if (err < h_nerr)
		return (h_errlist[err]);
	return ("Unknown resolver error");
}

/*! \file */