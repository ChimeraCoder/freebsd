
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

/* $Id: thread.c,v 1.17 2007/06/19 23:47:18 tbox Exp $ */

/*! \file */

#include <config.h>

#include <isc/thread.h>
#include <isc/util.h>

#ifndef THREAD_MINSTACKSIZE
#define THREAD_MINSTACKSIZE		(64U * 1024)
#endif

isc_result_t
isc_thread_create(isc_threadfunc_t func, isc_threadarg_t arg,
		  isc_thread_t *thread)
{
	pthread_attr_t attr;
	size_t stacksize;
	int ret;

	pthread_attr_init(&attr);

#if defined(HAVE_PTHREAD_ATTR_GETSTACKSIZE) && \
    defined(HAVE_PTHREAD_ATTR_SETSTACKSIZE)
	ret = pthread_attr_getstacksize(&attr, &stacksize);
	if (ret != 0)
		return (ISC_R_UNEXPECTED);

	if (stacksize < THREAD_MINSTACKSIZE) {
		ret = pthread_attr_setstacksize(&attr, THREAD_MINSTACKSIZE);
		if (ret != 0)
			return (ISC_R_UNEXPECTED);
	}
#endif

#if defined(PTHREAD_SCOPE_SYSTEM) && defined(NEED_PTHREAD_SCOPE_SYSTEM)
	ret = pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
	if (ret != 0)
		return (ISC_R_UNEXPECTED);
#endif

	ret = pthread_create(thread, &attr, func, arg);
	if (ret != 0)
		return (ISC_R_UNEXPECTED);

	pthread_attr_destroy(&attr);

	return (ISC_R_SUCCESS);
}

void
isc_thread_setconcurrency(unsigned int level) {
#if defined(CALL_PTHREAD_SETCONCURRENCY)
	(void)pthread_setconcurrency(level);
#else
	UNUSED(level);
#endif
}