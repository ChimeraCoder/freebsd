
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

#include "opt_kdb.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/eventhandler.h>
#include <sys/kdb.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/proc.h>
#include <sys/dtrace_bsd.h>
#include <sys/sysctl.h>

#define KDTRACE_PROC_SIZE	64
#define	KDTRACE_THREAD_SIZE	256

FEATURE(kdtrace_hooks,
    "Kernel DTrace hooks which are required to load DTrace kernel modules");

static MALLOC_DEFINE(M_KDTRACE, "kdtrace", "DTrace hooks");

/* Return the DTrace process data size compiled in the kernel hooks. */
size_t
kdtrace_proc_size()
{

	return (KDTRACE_PROC_SIZE);
}

static void
kdtrace_proc_ctor(void *arg __unused, struct proc *p)
{

	p->p_dtrace = malloc(KDTRACE_PROC_SIZE, M_KDTRACE, M_WAITOK|M_ZERO);
}

static void
kdtrace_proc_dtor(void *arg __unused, struct proc *p)
{

	if (p->p_dtrace != NULL) {
		free(p->p_dtrace, M_KDTRACE);
		p->p_dtrace = NULL;
	}
}

/* Return the DTrace thread data size compiled in the kernel hooks. */
size_t
kdtrace_thread_size()
{

	return (KDTRACE_THREAD_SIZE);
}

static void
kdtrace_thread_ctor(void *arg __unused, struct thread *td)
{

	td->td_dtrace = malloc(KDTRACE_THREAD_SIZE, M_KDTRACE, M_WAITOK|M_ZERO);
}

static void
kdtrace_thread_dtor(void *arg __unused, struct thread *td)
{

	if (td->td_dtrace != NULL) {
		free(td->td_dtrace, M_KDTRACE);
		td->td_dtrace = NULL;
	}
}

/*
 *  Initialise the kernel DTrace hooks.
 */
static void
init_dtrace(void *dummy __unused)
{

	EVENTHANDLER_REGISTER(process_ctor, kdtrace_proc_ctor, NULL,
	    EVENTHANDLER_PRI_ANY);
	EVENTHANDLER_REGISTER(process_dtor, kdtrace_proc_dtor, NULL,
	    EVENTHANDLER_PRI_ANY);
	EVENTHANDLER_REGISTER(thread_ctor, kdtrace_thread_ctor, NULL,
	    EVENTHANDLER_PRI_ANY);
	EVENTHANDLER_REGISTER(thread_dtor, kdtrace_thread_dtor, NULL,
	    EVENTHANDLER_PRI_ANY);
}

SYSINIT(kdtrace, SI_SUB_KDTRACE, SI_ORDER_FIRST, init_dtrace, NULL);