
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

#if defined(LIBC_SCCS) && !defined(lint)
static const char sccsid[] = "@(#)strerror.c	5.1 (Berkeley) 4/9/89";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>

#include <stdio.h>
#include <string.h>

#include "l_stdlib.h"

char *
strerror(
	int errnum
	)
{
	extern int sys_nerr;
	extern char *sys_errlist[];
	static char ebuf[20];

	if ((unsigned int)errnum < sys_nerr)
		return(sys_errlist[errnum]);
	(void)sprintf(ebuf, "Unknown error: %d", errnum);
	return(ebuf);
}
#else
int strerror_bs;
#endif