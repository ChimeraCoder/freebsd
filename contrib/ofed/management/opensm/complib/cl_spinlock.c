
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif				/* HAVE_CONFIG_H */

#include <complib/cl_spinlock.h>

void cl_spinlock_construct(IN cl_spinlock_t * const p_spinlock)
{
	CL_ASSERT(p_spinlock);

	p_spinlock->state = CL_UNINITIALIZED;
}

cl_status_t cl_spinlock_init(IN cl_spinlock_t * const p_spinlock)
{
	CL_ASSERT(p_spinlock);

	cl_spinlock_construct(p_spinlock);

	/* Initialize with pthread_mutexattr_t = NULL */
	if (pthread_mutex_init(&p_spinlock->mutex, NULL))
		return (CL_ERROR);

	p_spinlock->state = CL_INITIALIZED;
	return (CL_SUCCESS);
}

void cl_spinlock_destroy(IN cl_spinlock_t * const p_spinlock)
{
	CL_ASSERT(p_spinlock);
	CL_ASSERT(cl_is_state_valid(p_spinlock->state));

	if (p_spinlock->state == CL_INITIALIZED) {
		p_spinlock->state = CL_UNINITIALIZED;
		pthread_mutex_lock(&p_spinlock->mutex);
		pthread_mutex_unlock(&p_spinlock->mutex);
		pthread_mutex_destroy(&p_spinlock->mutex);
	}
	p_spinlock->state = CL_UNINITIALIZED;
}

void cl_spinlock_acquire(IN cl_spinlock_t * const p_spinlock)
{
	CL_ASSERT(p_spinlock);
	CL_ASSERT(p_spinlock->state == CL_INITIALIZED);

	pthread_mutex_lock(&p_spinlock->mutex);
}

void cl_spinlock_release(IN cl_spinlock_t * const p_spinlock)
{
	CL_ASSERT(p_spinlock);
	CL_ASSERT(p_spinlock->state == CL_INITIALIZED);

	pthread_mutex_unlock(&p_spinlock->mutex);
}