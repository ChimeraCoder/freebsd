
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

#include <stdio.h>

#include "spinlock.h"

long _atomic_lock_stub(volatile long *);
void _spinlock_stub(spinlock_t *);
void _spinunlock_stub(spinlock_t *);
void _spinlock_debug_stub(spinlock_t *, char *, int);

/*
 * Declare weak definitions in case the application is not linked
 * with libpthread.
 */
__weak_reference(_atomic_lock_stub, _atomic_lock);
__weak_reference(_spinlock_stub, _spinlock);
__weak_reference(_spinunlock_stub, _spinunlock);
__weak_reference(_spinlock_debug_stub, _spinlock_debug);

/*
 * This function is a stub for the _atomic_lock function in libpthread.
 */
long
_atomic_lock_stub(volatile long *lck __unused)
{
	return (0L);
}


/*
 * This function is a stub for the spinlock function in libpthread.
 */
void
_spinlock_stub(spinlock_t *lck __unused)
{
}

/*
 * This function is a stub for the spinunlock function in libpthread.
 */
void
_spinunlock_stub(spinlock_t *lck __unused)
{
}

/*
 * This function is a stub for the debug spinlock function in libpthread.
 */
void
_spinlock_debug_stub(spinlock_t *lck __unused, char *fname __unused, int lineno __unused)
{
}