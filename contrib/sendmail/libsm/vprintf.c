
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
SM_RCSID("@(#)$Id: vprintf.c,v 1.14 2001/09/11 04:04:49 gshapiro Exp $")
#include <sm/io.h>
#include "local.h"

/*
**  SM_VPRINTF -- print to standard out with variable length args
**
**	Parameters:
**		timeout -- length of time allow to do the print
**		fmt -- the format of the output
**		ap -- the variable number of args to be used for output
**
**	Returns:
**		as sm_io_vfprintf() does.
*/

int
sm_vprintf(timeout, fmt, ap)
	int timeout;
	char const *fmt;
	SM_VA_LOCAL_DECL
{
	return sm_io_vfprintf(smiostdout, timeout, fmt, ap);
}