
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
SM_RCSID("@(#)$Id: fput.c,v 1.20 2001/09/11 04:04:48 gshapiro Exp $")
#include <string.h>
#include <errno.h>
#include <sm/io.h>
#include <sm/assert.h>
#include "local.h"
#include "fvwrite.h"

/*
**  SM_IO_FPUTS -- add a string to the buffer for the file pointer
**
**	Parameters:
**		fp -- the file pointer for the buffer to be written to
**		timeout -- time to complete the put-string
**		s -- string to be placed in the buffer
**
**	Returns:
**		Failure: returns SM_IO_EOF
**		Success: returns 0 (zero)
*/

int
sm_io_fputs(fp, timeout, s)
	SM_FILE_T *fp;
	int timeout;
	const char *s;
{
	struct sm_uio uio;
	struct sm_iov iov;

	SM_REQUIRE_ISA(fp, SmFileMagic);
	iov.iov_base = (void *) s;
	iov.iov_len = uio.uio_resid = strlen(s);
	uio.uio_iov = &iov;
	uio.uio_iovcnt = 1;
	return sm_fvwrite(fp, timeout, &uio);
}