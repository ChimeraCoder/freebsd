
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
static char sccsid[] = "@(#)psignal.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * Print the name of the signal indicated
 * along with the supplied message.
 */
#include "namespace.h"
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "un-namespace.h"

void
psignal(sig, s)
	unsigned int sig;
	const char *s;
{
	const char *c;

	if (sig < NSIG)
		c = sys_siglist[sig];
	else
		c = "Unknown signal";
	if (s != NULL && *s != '\0') {
		(void)_write(STDERR_FILENO, s, strlen(s));
		(void)_write(STDERR_FILENO, ": ", 2);
	}
	(void)_write(STDERR_FILENO, c, strlen(c));
	(void)_write(STDERR_FILENO, "\n", 1);
}