
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
SM_RCSID("@(#)$Id: fwalk.c,v 1.21 2001/09/11 04:04:48 gshapiro Exp $")
#include <errno.h>
#include <sm/io.h>
#include "local.h"
#include "glue.h"

/*
**  SM_FWALK -- apply a function to all found-open file pointers
**
**	Parameters:
**		function -- a function vector to be applied
**		timeout -- time to complete actions (milliseconds)
**
**	Returns:
**		The (binary) OR'd result of each function call
*/

int
sm_fwalk(function, timeout)
	int (*function) __P((SM_FILE_T *, int *));
	int *timeout;
{
	register SM_FILE_T *fp;
	register int n, ret;
	register struct sm_glue *g;
	int fptimeout;

	ret = 0;
	for (g = &smglue; g != NULL; g = g->gl_next)
	{
		for (fp = g->gl_iobs, n = g->gl_niobs; --n >= 0; fp++)
		{
			if (fp->f_flags != 0)
			{
				if (*timeout == SM_TIME_DEFAULT)
					fptimeout = fp->f_timeout;
				else
					fptimeout = *timeout;
				if (fptimeout == SM_TIME_IMMEDIATE)
					continue; /* skip it */
				ret |= (*function)(fp, &fptimeout);
			}
		}
	}
	return ret;
}