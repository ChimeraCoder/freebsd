
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
SM_RCSID("@(#)$Id: fpurge.c,v 1.20 2001/09/11 04:04:48 gshapiro Exp $")
#include <stdlib.h>
#include <errno.h>
#include <sm/io.h>
#include <sm/assert.h>
#include "local.h"

/*
**  SM_IO_PURGE -- purge/empty the buffer without committing buffer content
**
**	Parameters:
**		fp -- file pointer to purge
**
**	Returns:
**		Failure: returns SM_IO_EOF and sets errno
**		Success: returns 0 (zero)
*/

int
sm_io_purge(fp)
	register SM_FILE_T *fp;
{
	SM_REQUIRE_ISA(fp, SmFileMagic);
	if (!fp->f_flags)
	{
		errno = EBADF;
		return SM_IO_EOF;
	}

	if (HASUB(fp))
		FREEUB(fp);
	fp->f_p = fp->f_bf.smb_base;
	fp->f_r = 0;

	/* implies SMFBF */
	fp->f_w = fp->f_flags & (SMLBF|SMNBF) ? 0 : fp->f_bf.smb_size;
	return 0;
}