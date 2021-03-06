
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

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)atexit.c	8.2 (Berkeley) 7/3/94";
#endif /* LIBC_SCCS and not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "namespace.h"
#include <link.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "atexit.h"
#include "un-namespace.h"

#include "libc_private.h"

#define	ATEXIT_FN_EMPTY	0
#define	ATEXIT_FN_STD	1
#define	ATEXIT_FN_CXA	2

static pthread_mutex_t atexit_mutex = PTHREAD_MUTEX_INITIALIZER;

#define _MUTEX_LOCK(x)		if (__isthreaded) _pthread_mutex_lock(x)
#define _MUTEX_UNLOCK(x)	if (__isthreaded) _pthread_mutex_unlock(x)
#define _MUTEX_DESTROY(x)	if (__isthreaded) _pthread_mutex_destroy(x)

struct atexit {
	struct atexit *next;			/* next in list */
	int ind;				/* next index in this table */
	struct atexit_fn {
		int fn_type;			/* ATEXIT_? from above */
		union {
			void (*std_func)(void);
			void (*cxa_func)(void *);
		} fn_ptr;			/* function pointer */
		void *fn_arg;			/* argument for CXA callback */
		void *fn_dso;			/* shared module handle */
	} fns[ATEXIT_SIZE];			/* the table itself */
};

static struct atexit *__atexit;		/* points to head of LIFO stack */

/*
 * Register the function described by 'fptr' to be called at application
 * exit or owning shared object unload time. This is a helper function
 * for atexit and __cxa_atexit.
 */
static int
atexit_register(struct atexit_fn *fptr)
{
	static struct atexit __atexit0;	/* one guaranteed table */
	struct atexit *p;

	_MUTEX_LOCK(&atexit_mutex);
	if ((p = __atexit) == NULL)
		__atexit = p = &__atexit0;
	else while (p->ind >= ATEXIT_SIZE) {
		struct atexit *old__atexit;
		old__atexit = __atexit;
	        _MUTEX_UNLOCK(&atexit_mutex);
		if ((p = (struct atexit *)malloc(sizeof(*p))) == NULL)
			return (-1);
		_MUTEX_LOCK(&atexit_mutex);
		if (old__atexit != __atexit) {
			/* Lost race, retry operation */
			_MUTEX_UNLOCK(&atexit_mutex);
			free(p);
			_MUTEX_LOCK(&atexit_mutex);
			p = __atexit;
			continue;
		}
		p->ind = 0;
		p->next = __atexit;
		__atexit = p;
	}
	p->fns[p->ind++] = *fptr;
	_MUTEX_UNLOCK(&atexit_mutex);
	return 0;
}

/*
 * Register a function to be performed at exit.
 */
int
atexit(void (*func)(void))
{
	struct atexit_fn fn;
	int error;

	fn.fn_type = ATEXIT_FN_STD;
	fn.fn_ptr.std_func = func;
	fn.fn_arg = NULL;
	fn.fn_dso = NULL;

 	error = atexit_register(&fn);	
	return (error);
}

/*
 * Register a function to be performed at exit or when an shared object
 * with given dso handle is unloaded dynamically.
 */
int
__cxa_atexit(void (*func)(void *), void *arg, void *dso)
{
	struct atexit_fn fn;
	int error;

	fn.fn_type = ATEXIT_FN_CXA;
	fn.fn_ptr.cxa_func = func;
	fn.fn_arg = arg;
	fn.fn_dso = dso;

 	error = atexit_register(&fn);	
	return (error);
}

#pragma weak __pthread_cxa_finalize
void __pthread_cxa_finalize(const struct dl_phdr_info *);

/*
 * Call all handlers registered with __cxa_atexit for the shared
 * object owning 'dso'.  Note: if 'dso' is NULL, then all remaining
 * handlers are called.
 */
void
__cxa_finalize(void *dso)
{
	struct dl_phdr_info phdr_info;
	struct atexit *p;
	struct atexit_fn fn;
	int n, has_phdr;

	if (dso != NULL)
		has_phdr = _rtld_addr_phdr(dso, &phdr_info);
	else
		has_phdr = 0;

	_MUTEX_LOCK(&atexit_mutex);
	for (p = __atexit; p; p = p->next) {
		for (n = p->ind; --n >= 0;) {
			if (p->fns[n].fn_type == ATEXIT_FN_EMPTY)
				continue; /* already been called */
			fn = p->fns[n];
			if (dso != NULL && dso != fn.fn_dso) {
				/* wrong DSO ? */
				if (!has_phdr || !__elf_phdr_match_addr(
				    &phdr_info, fn.fn_ptr.cxa_func))
					continue;
			}
			/*
			  Mark entry to indicate that this particular handler
			  has already been called.
			*/
			p->fns[n].fn_type = ATEXIT_FN_EMPTY;
		        _MUTEX_UNLOCK(&atexit_mutex);
		
			/* Call the function of correct type. */
			if (fn.fn_type == ATEXIT_FN_CXA)
				fn.fn_ptr.cxa_func(fn.fn_arg);
			else if (fn.fn_type == ATEXIT_FN_STD)
				fn.fn_ptr.std_func();
			_MUTEX_LOCK(&atexit_mutex);
		}
	}
	_MUTEX_UNLOCK(&atexit_mutex);
	if (dso == NULL)
		_MUTEX_DESTROY(&atexit_mutex);

	if (has_phdr && &__pthread_cxa_finalize != NULL)
		__pthread_cxa_finalize(&phdr_info);
}