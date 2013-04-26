
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

#include <stdio.h>
#include <unistd.h>
#include <complib/cl_thread.h>

/*
 * Internal function to run a new user mode thread.
 * This function is always run as a result of creation a new user mode thread.
 * Its main job is to synchronize the creation and running of the new thread.
 */
static void *__cl_thread_wrapper(void *arg)
{
	cl_thread_t *p_thread = (cl_thread_t *) arg;

	CL_ASSERT(p_thread);
	CL_ASSERT(p_thread->pfn_callback);

	p_thread->pfn_callback((void *)p_thread->context);

	return (NULL);
}

void cl_thread_construct(IN cl_thread_t * const p_thread)
{
	CL_ASSERT(p_thread);

	p_thread->osd.state = CL_UNINITIALIZED;
}

cl_status_t
cl_thread_init(IN cl_thread_t * const p_thread,
	       IN cl_pfn_thread_callback_t pfn_callback,
	       IN const void *const context, IN const char *const name)
{
	int ret;

	CL_ASSERT(p_thread);

	cl_thread_construct(p_thread);

	/* Initialize the thread structure */
	p_thread->pfn_callback = pfn_callback;
	p_thread->context = context;

	ret = pthread_create(&p_thread->osd.id, NULL,
			     __cl_thread_wrapper, (void *)p_thread);

	if (ret != 0)		/* pthread_create returns a "0" for success */
		return (CL_ERROR);

	p_thread->osd.state = CL_INITIALIZED;

	return (CL_SUCCESS);
}

void cl_thread_destroy(IN cl_thread_t * const p_thread)
{
	CL_ASSERT(p_thread);
	CL_ASSERT(cl_is_state_valid(p_thread->osd.state));

	if (p_thread->osd.state == CL_INITIALIZED)
		pthread_join(p_thread->osd.id, NULL);

	p_thread->osd.state = CL_UNINITIALIZED;
}

void cl_thread_suspend(IN const uint32_t pause_ms)
{
	/* Convert to micro seconds */
	usleep(pause_ms * 1000);
}

void cl_thread_stall(IN const uint32_t pause_us)
{
	/*
	 * Not quite a busy wait, but Linux is lacking in terms of high
	 * resolution time stamp information in user mode.
	 */
	usleep(pause_us);
}

int cl_proc_count(void)
{
	uint32_t ret;

	ret = sysconf(_SC_NPROCESSORS_ONLN);
	if (!ret)
		return 1;	/* Workaround for PPC where get_nprocs() returns 0 */

	return ret;
}

boolean_t cl_is_current_thread(IN const cl_thread_t * const p_thread)
{
	pthread_t current;

	CL_ASSERT(p_thread);
	CL_ASSERT(p_thread->osd.state == CL_INITIALIZED);

	current = pthread_self();
	return (pthread_equal(current, p_thread->osd.id));
}