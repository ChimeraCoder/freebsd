
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
static char sccsid[] = "@(#)tmpnam.c	8.3 (Berkeley) 3/28/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <stdio.h>
#include <unistd.h>

__warn_references(tmpnam,
    "warning: tmpnam() possibly used unsafely; consider using mkstemp()");

extern char *_mktemp(char *);

char *
tmpnam(char *s)
{
	static u_long tmpcount;
	static char buf[L_tmpnam];

	if (s == NULL)
		s = buf;
	(void)snprintf(s, L_tmpnam, "%stmp.%lu.XXXXXX", P_tmpdir, tmpcount);
	++tmpcount;
	return (_mktemp(s));
}