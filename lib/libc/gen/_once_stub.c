
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

#include "namespace.h"
#include <pthread.h>
#include "un-namespace.h"
#include "libc_private.h"

/* This implements pthread_once() for the single-threaded case. */
static int
_libc_once(pthread_once_t *once_control, void (*init_routine)(void))
{

	if (once_control->state == PTHREAD_DONE_INIT)
		return (0);
	init_routine();
	once_control->state = PTHREAD_DONE_INIT;
	return (0);
}

/*
 * This is the internal interface provided to libc.  It will use
 * pthread_once() from the threading library in a multi-threaded
 * process and _libc_once() for a single-threaded library.  Because
 * _libc_once() uses the same ABI for the values in the pthread_once_t
 * structure as the threading library, it is safe for a process to
 * switch from _libc_once() to pthread_once() when threading is
 * enabled.
 */
int
_once(pthread_once_t *once_control, void (*init_routine)(void))
{

	if (__isthreaded)
		return (_pthread_once(once_control, init_routine));
	return (_libc_once(once_control, init_routine));
}