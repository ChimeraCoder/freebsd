
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
SM_RCSID("@(#)$Id: clrerr.c,v 1.13 2001/09/11 04:04:48 gshapiro Exp $")
#include <sm/io.h>
#include <sm/assert.h>
#include "local.h"

/*
**  SM_IO_CLEARERR -- public function to clear a file pointer's error status
**
**	Parameters:
**		fp -- the file pointer
**
**	Returns:
**		nothing.
*/
#undef	sm_io_clearerr

void
sm_io_clearerr(fp)
	SM_FILE_T *fp;
{
	SM_REQUIRE_ISA(fp, SmFileMagic);

	sm_clearerr(fp);
}