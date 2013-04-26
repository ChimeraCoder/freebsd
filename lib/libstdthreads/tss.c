
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

#include <pthread.h>

#include "threads.h"

int
tss_create(tss_t *key, tss_dtor_t dtor)
{

	if (pthread_key_create(key, dtor) != 0)
		return (thrd_error);
	return (thrd_success);
}

void
tss_delete(tss_t key)
{

	(void)pthread_key_delete(key);
}

void *
tss_get(tss_t key)
{

	return (pthread_getspecific(key));
}

int
tss_set(tss_t key, void *val)
{

	if (pthread_setspecific(key, val) != 0)
		return (thrd_error);
	return (thrd_success);
}

_Static_assert(TSS_DTOR_ITERATIONS == PTHREAD_DESTRUCTOR_ITERATIONS,
    "TSS_DTOR_ITERATIONS must be identical to PTHREAD_DESTRUCTOR_ITERATIONS");