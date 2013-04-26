
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
static char sccsid[] = "@(#)fvwrite.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "local.h"
#include "fvwrite.h"

/*
 * Write some memory regions.  Return zero on success, EOF on error.
 *
 * This routine is large and unsightly, but most of the ugliness due
 * to the three different kinds of output buffering is handled here.
 */
int
__sfvwrite(FILE *fp, struct __suio *uio)
{
	size_t len;
	char *p;
	struct __siov *iov;
	int w, s;
	char *nl;
	int nlknown, nldist;

	if (uio->uio_resid == 0)
		return (0);
	/* make sure we can write */
	if (prepwrite(fp) != 0)
		return (EOF);

#define	MIN(a, b) ((a) < (b) ? (a) : (b))
#define	COPY(n)	  (void)memcpy((void *)fp->_p, (void *)p, (size_t)(n))

	iov = uio->uio_iov;
	p = iov->iov_base;
	len = iov->iov_len;
	iov++;
#define GETIOV(extra_work) \
	while (len == 0) { \
		extra_work; \
		p = iov->iov_base; \
		len = iov->iov_len; \
		iov++; \
	}
	if (fp->_flags & __SNBF) {
		/*
		 * Unbuffered: write up to BUFSIZ bytes at a time.
		 */
		do {
			GETIOV(;);
			w = _swrite(fp, p, MIN(len, BUFSIZ));
			if (w <= 0)
				goto err;
			p += w;
			len -= w;
		} while ((uio->uio_resid -= w) != 0);
	} else if ((fp->_flags & __SLBF) == 0) {
		/*
		 * Fully buffered: fill partially full buffer, if any,
		 * and then flush.  If there is no partial buffer, write
		 * one _bf._size byte chunk directly (without copying).
		 *
		 * String output is a special case: write as many bytes
		 * as fit, but pretend we wrote everything.  This makes
		 * snprintf() return the number of bytes needed, rather
		 * than the number used, and avoids its write function
		 * (so that the write function can be invalid).
		 */
		do {
			GETIOV(;);
			if ((fp->_flags & (__SALC | __SSTR)) ==
			    (__SALC | __SSTR) && fp->_w < len) {
				size_t blen = fp->_p - fp->_bf._base;

				/*
				 * Alloc an extra 128 bytes (+ 1 for NULL)
				 * so we don't call realloc(3) so often.
				 */
				fp->_w = len + 128;
				fp->_bf._size = blen + len + 128;
				fp->_bf._base =
				    reallocf(fp->_bf._base, fp->_bf._size + 1);
				if (fp->_bf._base == NULL)
					goto err;
				fp->_p = fp->_bf._base + blen;
			}
			w = fp->_w;
			if (fp->_flags & __SSTR) {
				if (len < w)
					w = len;
				if (w > 0) {
					COPY(w);        /* copy MIN(fp->_w,len), */
					fp->_w -= w;
					fp->_p += w;
				}
				w = len;	/* but pretend copied all */
			} else if (fp->_p > fp->_bf._base && len > w) {
				/* fill and flush */
				COPY(w);
				/* fp->_w -= w; */ /* unneeded */
				fp->_p += w;
				if (__fflush(fp))
					goto err;
			} else if (len >= (w = fp->_bf._size)) {
				/* write directly */
				w = _swrite(fp, p, w);
				if (w <= 0)
					goto err;
			} else {
				/* fill and done */
				w = len;
				COPY(w);
				fp->_w -= w;
				fp->_p += w;
			}
			p += w;
			len -= w;
		} while ((uio->uio_resid -= w) != 0);
	} else {
		/*
		 * Line buffered: like fully buffered, but we
		 * must check for newlines.  Compute the distance
		 * to the first newline (including the newline),
		 * or `infinity' if there is none, then pretend
		 * that the amount to write is MIN(len,nldist).
		 */
		nlknown = 0;
		nldist = 0;	/* XXX just to keep gcc happy */
		do {
			GETIOV(nlknown = 0);
			if (!nlknown) {
				nl = memchr((void *)p, '\n', len);
				nldist = nl ? nl + 1 - p : len + 1;
				nlknown = 1;
			}
			s = MIN(len, nldist);
			w = fp->_w + fp->_bf._size;
			if (fp->_p > fp->_bf._base && s > w) {
				COPY(w);
				/* fp->_w -= w; */
				fp->_p += w;
				if (__fflush(fp))
					goto err;
			} else if (s >= (w = fp->_bf._size)) {
				w = _swrite(fp, p, w);
				if (w <= 0)
				 	goto err;
			} else {
				w = s;
				COPY(w);
				fp->_w -= w;
				fp->_p += w;
			}
			if ((nldist -= w) == 0) {
				/* copied the newline: flush and forget */
				if (__fflush(fp))
					goto err;
				nlknown = 0;
			}
			p += w;
			len -= w;
		} while ((uio->uio_resid -= w) != 0);
	}
	return (0);

err:
	fp->_flags |= __SERR;
	return (EOF);
}