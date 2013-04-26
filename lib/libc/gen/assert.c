
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
static char sccsid[] = "@(#)assert.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

void
__assert(func, file, line, failedexpr)
	const char *func, *file;
	int line;
	const char *failedexpr;
{
	if (func == NULL)
		(void)fprintf(stderr,
		     "Assertion failed: (%s), file %s, line %d.\n", failedexpr,
		     file, line);
	else
		(void)fprintf(stderr,
		     "Assertion failed: (%s), function %s, file %s, line %d.\n",
		     failedexpr, func, file, line);
	abort();
	/* NOTREACHED */
}