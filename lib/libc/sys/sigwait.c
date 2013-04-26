
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <errno.h>
#include <signal.h>

int __sys_sigwait(const sigset_t * restrict, int * restrict);

__weak_reference(__sigwait, sigwait);

int
__sigwait(const sigset_t * restrict set, int * restrict sig)
{
	int ret;

	/* POSIX does not allow EINTR to be returned */
	do  {
		ret = __sys_sigwait(set, sig);
	} while (ret == EINTR);
	return (ret);
}