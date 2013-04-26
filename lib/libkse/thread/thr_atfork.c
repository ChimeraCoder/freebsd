
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
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/queue.h>
#include "un-namespace.h"
#include "thr_private.h"

__weak_reference(_pthread_atfork, pthread_atfork);

int
_pthread_atfork(void (*prepare)(void), void (*parent)(void),
    void (*child)(void))
{
	struct pthread_atfork *af;

	if (_thr_initial == NULL)
		_libpthread_init(NULL);

	if ((af = malloc(sizeof(struct pthread_atfork))) == NULL)
		return (ENOMEM);

	af->prepare = prepare;
	af->parent = parent;
	af->child = child;
	_pthread_mutex_lock(&_thr_atfork_mutex);
	TAILQ_INSERT_TAIL(&_thr_atfork_list, af, qe);
	_pthread_mutex_unlock(&_thr_atfork_mutex);
	return (0);
}