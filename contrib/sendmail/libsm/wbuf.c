
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

#include <sm/gen.h>
SM_RCSID("@(#)$Id: wbuf.c,v 1.21 2001/09/11 04:04:49 gshapiro Exp $")
#include <errno.h>
#include <sm/io.h>
#include "local.h"

/* Note: This function is called from a macro located in <sm/io.h> */

/*
**  SM_WBUF -- write character to and flush (likely now full) buffer
**
**  Write the given character into the (probably full) buffer for
**  the given file.  Flush the buffer out if it is or becomes full,
**  or if c=='\n' and the file is line buffered.
**
**	Parameters:
**		fp -- the file pointer
**		timeout -- time to complete operation (milliseconds)
**		c -- int representation of the character to add
**
**	Results:
**		Failure: -1 and sets errno
**		Success: int value of 'c'
*/

int
sm_wbuf(fp, timeout, c)
	register SM_FILE_T *fp;
	int timeout;
	register int c;
{
	register int n;

	/*
	**  In case we cannot write, or longjmp takes us out early,
	**  make sure w is 0 (if fully- or un-buffered) or -bf.smb_size
	**  (if line buffered) so that we will get called again.
	**  If we did not do this, a sufficient number of sm_io_putc()
	**  calls might wrap w from negative to positive.
	*/

	fp->f_w = fp->f_lbfsize;
	if (cantwrite(fp))
	{
		errno = EBADF;
		return SM_IO_EOF;
	}
	c = (unsigned char)c;

	/*
	**  If it is completely full, flush it out.  Then, in any case,
	**  stuff c into the buffer.  If this causes the buffer to fill
	**  completely, or if c is '\n' and the file is line buffered,
	**  flush it (perhaps a second time).  The second flush will always
	**  happen on unbuffered streams, where bf.smb_size==1; sm_io_flush()
	**  guarantees that sm_io_putc() will always call sm_wbuf() by setting
	**  w to 0, so we need not do anything else.
	**  Note for the timeout, only one of the sm_io_flush's will get called.
	*/

	n = fp->f_p - fp->f_bf.smb_base;
	if (n >= fp->f_bf.smb_size)
	{
		if (sm_io_flush(fp, timeout))
			return SM_IO_EOF; /* sm_io_flush() sets errno */
		n = 0;
	}
	fp->f_w--;
	*fp->f_p++ = c;
	if (++n == fp->f_bf.smb_size || (fp->f_flags & SMLBF && c == '\n'))
		if (sm_io_flush(fp, timeout))
			return SM_IO_EOF; /* sm_io_flush() sets errno */
	return c;
}