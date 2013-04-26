
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

#include <sys/types.h>
#include <sys/ucontext.h>
#include <stdarg.h>
#include <stdlib.h>

typedef void (*func_t)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t,
    uint64_t);

/* Prototypes */
static void makectx_wrapper(ucontext_t *ucp, func_t func, uint64_t *args);

__weak_reference(__makecontext, makecontext);

void
__makecontext(ucontext_t *ucp, void (*start)(void), int argc, ...)
{
	uint64_t *args;
	uint64_t *sp;
	va_list ap;
	int i;

	/* A valid context is required. */
	if ((ucp == NULL) || (ucp->uc_mcontext.mc_len != sizeof(mcontext_t)))
		return;
	else if ((argc < 0) || (argc > 6) || (ucp->uc_stack.ss_sp == NULL) ||
	    (ucp->uc_stack.ss_size < MINSIGSTKSZ)) {
		/*
		 * This should really return -1 with errno set to ENOMEM
		 * or something, but the spec says that makecontext is
		 * a void function.   At least make sure that the context
		 * isn't valid so it can't be used without an error.
		 */
		ucp->uc_mcontext.mc_len = 0;
		return;
	}

	/* Align the stack to 16 bytes. */
	sp = (uint64_t *)(ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size);
	sp = (uint64_t *)((uint64_t)sp & ~15UL);

	/* Allocate space for a maximum of 6 arguments on the stack. */
	args = sp - 6;

	/*
	 * Account for arguments on stack and do the funky C entry alignment.
	 * This means that we need an 8-byte-odd alignment since the ABI expects
	 * the return address to be pushed, thus breaking the 16 byte alignment.
	 */
	sp -= 7;

	/* Add the arguments: */
	va_start(ap, argc);
	for (i = 0; i < argc; i++)
		args[i] = va_arg(ap, uint64_t);
	va_end(ap);
	for (i = argc; i < 6; i++)
		args[i] = 0;

	ucp->uc_mcontext.mc_rdi = (register_t)ucp;
	ucp->uc_mcontext.mc_rsi = (register_t)start;
	ucp->uc_mcontext.mc_rdx = (register_t)args;
	ucp->uc_mcontext.mc_rbp = 0;
	ucp->uc_mcontext.mc_rbx = (register_t)sp;
	ucp->uc_mcontext.mc_rsp = (register_t)sp;
	ucp->uc_mcontext.mc_rip = (register_t)makectx_wrapper;
}

static void
makectx_wrapper(ucontext_t *ucp, func_t func, uint64_t *args)
{
	(*func)(args[0], args[1], args[2], args[3], args[4], args[5]);
	if (ucp->uc_link == NULL)
		exit(0);
	setcontext((const ucontext_t *)ucp->uc_link);
	/* should never get here */
	abort();
	/* NOTREACHED */
}