
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
static char sccsid[] = "@(#)wbuf.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <stdio.h>
#include "local.h"

/*
 * Write the given character into the (probably full) buffer for
 * the given file.  Flush the buffer out if it is or becomes full,
 * or if c=='\n' and the file is line buffered.
 *
 * Non-MT-safe
 */
int
__swbuf(int c, FILE *fp)
{
	int n;

	/*
	 * In case we cannot write, or longjmp takes us out early,
	 * make sure _w is 0 (if fully- or un-buffered) or -_bf._size
	 * (if line buffered) so that we will get called again.
	 * If we did not do this, a sufficient number of putc()
	 * calls might wrap _w from negative to positive.
	 */
	fp->_w = fp->_lbfsize;
	if (prepwrite(fp) != 0)
		return (EOF);
	c = (unsigned char)c;

	ORIENT(fp, -1);

	/*
	 * If it is completely full, flush it out.  Then, in any case,
	 * stuff c into the buffer.  If this causes the buffer to fill
	 * completely, or if c is '\n' and the file is line buffered,
	 * flush it (perhaps a second time).  The second flush will always
	 * happen on unbuffered streams, where _bf._size==1; fflush()
	 * guarantees that putc() will always call wbuf() by setting _w
	 * to 0, so we need not do anything else.
	 */
	n = fp->_p - fp->_bf._base;
	if (n >= fp->_bf._size) {
		if (__fflush(fp))
			return (EOF);
		n = 0;
	}
	fp->_w--;
	*fp->_p++ = c;
	if (++n == fp->_bf._size || (fp->_flags & __SLBF && c == '\n'))
		if (__fflush(fp))
			return (EOF);
	return (c);
}