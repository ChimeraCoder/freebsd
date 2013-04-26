
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
SM_RCSID("@(#)$Id: strerror.c,v 1.23 2001/09/11 04:04:49 gshapiro Exp $")

/*
**  define strerror for platforms that lack it.
*/

#include <errno.h>
#include <stdio.h>	/* sys_errlist, on some platforms */

#include <sm/io.h>	/* sm_snprintf */
#include <sm/string.h>
#include <sm/conf.h>
#include <sm/errstring.h>

#if !defined(ERRLIST_PREDEFINED)
extern char *sys_errlist[];
extern int sys_nerr;
#endif /* !defined(ERRLIST_PREDEFINED) */

#if !HASSTRERROR

/*
**  STRERROR -- return error message string corresponding to an error number.
**
**	Parameters:
**		err -- error number.
**
**	Returns:
**		Error string (might be pointer to static buffer).
*/

char *
strerror(err)
	int err;
{
	static char buf[64];

	if (err >= 0 && err < sys_nerr)
		return (char *) sys_errlist[err];
	else
	{
		(void) sm_snprintf(buf, sizeof(buf), "Error %d", err);
		return buf;
	}
}
#endif /* !HASSTRERROR */