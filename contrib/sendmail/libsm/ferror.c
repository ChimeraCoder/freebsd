
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
SM_RCSID("@(#)$Id: ferror.c,v 1.13 2001/09/11 04:04:48 gshapiro Exp $")
#include <sm/io.h>
#include <sm/assert.h>
#include "local.h"

/*
**  SM_IO_ERROR -- subroutine version of the macro sm_io_error.
**
**	Parameters:
**		fp -- file pointer
**
**	Returns:
**		0 (zero) when 'fp' is not in an error state
**		non-zero when 'fp' is in an error state
*/

#undef sm_io_error

int
sm_io_error(fp)
	SM_FILE_T *fp;
{
	SM_REQUIRE_ISA(fp, SmFileMagic);

	return sm_error(fp);
}