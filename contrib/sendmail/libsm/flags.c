
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
SM_RCSID("@(#)$Id: flags.c,v 1.23 2006/12/19 19:44:23 ca Exp $")
#include <sys/types.h>
#include <sys/file.h>
#include <errno.h>
#include <sm/io.h>
#include "local.h"

/*
**  SM_FLAGS -- translate external (user) flags into internal flags
**
**	Paramters:
**		flags -- user select flags
**
**	Returns:
**		Internal flag value matching user selected flags
*/

int
sm_flags(flags)
	int flags;
{
	int ret;

	switch(SM_IO_MODE(flags))
	{
	  case SM_IO_RDONLY:	/* open for reading */
		ret = SMRD;
		break;

	  case SM_IO_WRONLY:	/* open for writing */
		ret = SMWR;
		break;

	  case SM_IO_APPEND:	/* open for appending */
		ret = SMWR;
		break;

	  case SM_IO_RDWR:	/* open for read and write */
		ret = SMRW;
		break;

	  default:
		ret = 0;
		break;
	}
	if (SM_IS_BINARY(flags))
		ret |= SM_IO_BINARY;
	return ret;
}