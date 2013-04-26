
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
SM_RCSID("@(#)$Id: get.c,v 1.18 2001/09/11 04:04:48 gshapiro Exp $")
#include <sm/io.h>
#include <sm/assert.h>
#include "local.h"

/*
**  SM_IO_GETC -- get a character from a file
**
**	Parameters:
**		fp -- the file to get the character from
**		timeout -- time to complete getc
**
**	Returns:
**		Success: the value of the character read.
**		Failure: SM_IO_EOF
**
**	This is a function version of the macro (in <sm/io.h>).
**	It is guarded with locks (which are currently not functional)
**	for multi-threaded programs.
*/

#undef sm_io_getc

int
sm_io_getc(fp, timeout)
	register SM_FILE_T *fp;
	int timeout;
{
	SM_REQUIRE_ISA(fp, SmFileMagic);
	return sm_getc(fp, timeout);
}