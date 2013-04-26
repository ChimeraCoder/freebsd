
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
SM_RCSID("@(#)$Id: fscanf.c,v 1.17 2001/09/11 04:04:48 gshapiro Exp $")
#include <sm/varargs.h>
#include <sm/assert.h>
#include <sm/io.h>
#include "local.h"

/*
**  SM_IO_FSCANF -- convert input data to translated format
**
**	Parameters:
**		fp -- the file pointer to obtain the data from
**		timeout -- time to complete scan
**		fmt -- the format to translate the data to
**		... -- memory locations to place the formated data
**
**	Returns:
**		Failure: returns SM_IO_EOF
**		Success: returns the number of data units translated
*/

int
#if SM_VA_STD
sm_io_fscanf(SM_FILE_T *fp, int timeout, char const *fmt, ...)
#else /* SM_VA_STD */
sm_io_fscanf(fp, timeout, fmt, va_alist)
	SM_FILE_T *fp;
	int timeout;
	char *fmt;
	va_dcl
#endif /* SM_VA_STD */
{
	int ret;
	SM_VA_LOCAL_DECL

	SM_REQUIRE_ISA(fp, SmFileMagic);
	SM_VA_START(ap, fmt);
	ret = sm_vfscanf(fp, timeout, fmt, ap);
	SM_VA_END(ap);
	return ret;
}