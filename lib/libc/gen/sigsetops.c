
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
static char sccsid[] = "@(#)sigsetops.c	8.1 (Berkeley) 6/4/93";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <errno.h>
#include <signal.h>

int
sigaddset(set, signo)
	sigset_t *set;
	int signo;
{

	if (signo <= 0 || signo > _SIG_MAXSIG) {
		errno = EINVAL;
		return (-1);
	}
	set->__bits[_SIG_WORD(signo)] |= _SIG_BIT(signo);
	return (0);
}

int
sigdelset(set, signo)
	sigset_t *set;
	int signo;
{

	if (signo <= 0 || signo > _SIG_MAXSIG) {
		errno = EINVAL;
		return (-1);
	}
	set->__bits[_SIG_WORD(signo)] &= ~_SIG_BIT(signo);
	return (0);
}

int
sigemptyset(set)
	sigset_t *set;
{
	int i;

	for (i = 0; i < _SIG_WORDS; i++)
		set->__bits[i] = 0;
	return (0);
}

int
sigfillset(set)
	sigset_t *set;
{
	int i;

	for (i = 0; i < _SIG_WORDS; i++)
		set->__bits[i] = ~0U;
	return (0);
}

int
sigismember(set, signo)
	const sigset_t *set;
	int signo;
{

	if (signo <= 0 || signo > _SIG_MAXSIG) {
		errno = EINVAL;
		return (-1);
	}
	return ((set->__bits[_SIG_WORD(signo)] & _SIG_BIT(signo)) ? 1 : 0);
}