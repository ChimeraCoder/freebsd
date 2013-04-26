
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
SM_RCSID("@(#)$Id: fwrite.c,v 1.24 2001/09/11 04:04:48 gshapiro Exp $")
#include <errno.h>
#include <sm/io.h>
#include <sm/assert.h>
#include "local.h"
#include "fvwrite.h"

/*
**  SM_IO_WRITE -- write to a file pointer
**
**	Parameters:
**		fp -- file pointer writing to
**		timeout -- time to complete the write
**		buf -- location of data to be written
**		size -- number of bytes to be written
**
**	Result:
**		Failure: returns 0 _and_ sets errno
**		Success: returns >=0 with errno unchanged, where the
**			number returned is the number of bytes written.
*/

size_t
sm_io_write(fp, timeout, buf, size)
	SM_FILE_T *fp;
	int timeout;
	const void *buf;
	size_t size;
{
	struct sm_uio uio;
	struct sm_iov iov;

	SM_REQUIRE_ISA(fp, SmFileMagic);

	if (fp->f_write == NULL)
	{
		errno = ENODEV;
		return 0;
	}

	iov.iov_base = (void *) buf;
	uio.uio_resid = iov.iov_len = size;
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;

	/* The usual case is success (sm_fvwrite returns 0) */
	if (sm_fvwrite(fp, timeout, &uio) == 0)
		return size;

	/* else return number of bytes actually written */
	return size - uio.uio_resid;
}