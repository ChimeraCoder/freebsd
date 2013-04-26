
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
SM_RCSID("@(#)$Id: rewind.c,v 1.18 2001/09/11 04:04:49 gshapiro Exp $")
#include <errno.h>
#include <sm/io.h>
#include <sm/assert.h>
#include "local.h"

/*
**  SM_IO_REWIND -- rewind the file
**
**	Seeks the file to the begining and clears any outstanding errors.
**
**	Parameters:
**		fp -- the flie pointer for rewind
**		timeout -- time to complete the rewind
**
**	Returns:
**		none.
*/

void
sm_io_rewind(fp, timeout)
	register SM_FILE_T *fp;
	int timeout;
{
	SM_REQUIRE_ISA(fp, SmFileMagic);
	(void) sm_io_seek(fp, timeout, 0L, SM_IO_SEEK_SET);
	(void) sm_io_clearerr(fp);
	errno = 0;      /* not required, but seems reasonable */
}