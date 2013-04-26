
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

#include "namespace.h"
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_pthread_cleanup_push, pthread_cleanup_push);
__weak_reference(_pthread_cleanup_pop, pthread_cleanup_pop);

void
_pthread_cleanup_push(void (*routine) (void *), void *routine_arg)
{
	struct pthread	*curthread = _get_curthread();
	struct pthread_cleanup *new;

	if ((new = (struct pthread_cleanup *)
	    malloc(sizeof(struct pthread_cleanup))) != NULL) {
		new->routine = routine;
		new->routine_arg = routine_arg;
		new->onstack = 0;
		new->next = curthread->cleanup;

		curthread->cleanup = new;
	}
}

void
_pthread_cleanup_pop(int execute)
{
	struct pthread	*curthread = _get_curthread();
	struct pthread_cleanup *old;

	if ((old = curthread->cleanup) != NULL) {
		curthread->cleanup = old->next;
		if (execute) {
			old->routine(old->routine_arg);
		}
		if (old->onstack == 0)
			free(old);
	}
}