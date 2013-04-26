
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
static char sccsid[] = "@(#)gets.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/cdefs.h>
#include "un-namespace.h"
#include "libc_private.h"
#include "local.h"

__warn_references(gets, "warning: this program uses gets(), which is unsafe.");

char *
gets(char *buf)
{
	int c;
	char *s;
	static int warned;
	static char w[] =
	    "warning: this program uses gets(), which is unsafe.\n";

	FLOCKFILE(stdin);
	ORIENT(stdin, -1);
	if (!warned) {
		(void) _write(STDERR_FILENO, w, sizeof(w) - 1);
		warned = 1;
	}
	for (s = buf; (c = __sgetc(stdin)) != '\n';)
		if (c == EOF)
			if (s == buf) {
				FUNLOCKFILE(stdin);
				return (NULL);
			} else
				break;
		else
			*s++ = c;
	*s = 0;
	FUNLOCKFILE(stdin);
	return (buf);
}