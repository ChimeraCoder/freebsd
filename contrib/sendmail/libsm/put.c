
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
SM_RCSID("@(#)$Id: put.c,v 1.27 2001/12/19 05:19:35 ca Exp $")
#include <string.h>
#include <errno.h>
#include <sm/io.h>
#include <sm/assert.h>
#include <sm/errstring.h>
#include <sm/string.h>
#include "local.h"
#include "fvwrite.h"

/*
**  SM_IO_PUTC -- output a character to the file
**
**  Function version of the macro sm_io_putc (in <sm/io.h>).
**
**	Parameters:
**		fp -- file to output to
**		timeout -- time to complete putc
**		c -- int value of character to output
**
**	Returns:
**		Failure: returns SM_IO_EOF _and_ sets errno
**		Success: returns sm_putc() value.
**
*/

#undef sm_io_putc

int
sm_io_putc(fp, timeout, c)
	SM_FILE_T *fp;
	int timeout;
	int c;
{
	SM_REQUIRE_ISA(fp, SmFileMagic);
	if (cantwrite(fp))
	{
		errno = EBADF;
		return SM_IO_EOF;
	}
	return sm_putc(fp, timeout, c);
}


/*
**  SM_PERROR -- print system error messages to smioerr
**
**	Parameters:
**		s -- message to print
**
**	Returns:
**		none
*/

void
sm_perror(s)
	const char *s;
{
	int save_errno = errno;

	if (s != NULL && *s != '\0')
		(void) sm_io_fprintf(smioerr, SM_TIME_DEFAULT, "%s: ", s);
	(void) sm_io_fprintf(smioerr, SM_TIME_DEFAULT, "%s\n",
			     sm_errstring(save_errno));
}