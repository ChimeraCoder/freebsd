
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
#include <machine/fpu.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct fdesc {
	uint64_t ip;
	uint64_t gp;
};

typedef void (*func_t)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t,
    uint64_t, uint64_t, uint64_t);

static __inline uint64_t *
spill(uint64_t *bsp, uint64_t arg)
{
	*bsp++ = arg;
	if (((intptr_t)bsp & 0x1ff) == 0x1f8)
		*bsp++ = 0;
	return (bsp);
}

static void
ctx_wrapper(ucontext_t *ucp, func_t func, uint64_t *args)
{

	(*func)(args[0], args[1], args[2], args[3], args[4], args[5], args[6],
	    args[7]);
	if (ucp->uc_link == NULL)
		exit(0);
	setcontext((const ucontext_t *)ucp->uc_link);
	/* should never get here */
	abort();
	/* NOTREACHED */
}

__weak_reference(__makecontext, makecontext);

void
__makecontext(ucontext_t *ucp, void (*func)(void), int argc, ...)
{
	uint64_t *args, *bsp;
	va_list ap;
	int i;

	/*
	 * Drop the ball completely if something's not right. We only
	 * support general registers as arguments and not more than 8
	 * of them. Things get hairy if we need to support FP registers
	 * (alignment issues) or more than 8 arguments (stack based).
	 */
	if (argc < 0 || argc > 8 || ucp == NULL ||
	    ucp->uc_stack.ss_sp == NULL || (ucp->uc_stack.ss_size & 15) ||
	    ((intptr_t)ucp->uc_stack.ss_sp & 15) ||
	    ucp->uc_stack.ss_size < MINSIGSTKSZ)
		abort();

	/*
	 * Copy the arguments of function 'func' onto the (memory) stack.
	 * Always take up space for 8 arguments.
	 */
	va_start(ap, argc);
	args = (uint64_t*)(ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size) - 8;
	i = 0;
	while (i < argc)
		args[i++] = va_arg(ap, uint64_t);
	while (i < 8)
		args[i++] = 0;
	va_end(ap);

	/*
	 * Push (spill) the arguments of the context wrapper onto the register
	 * stack. They get loaded by the RSE on a context switch.
	 */
	bsp = (uint64_t*)ucp->uc_stack.ss_sp;
	bsp = spill(bsp, (intptr_t)ucp);
	bsp = spill(bsp, (intptr_t)func);
	bsp = spill(bsp, (intptr_t)args);

	/*
	 * Setup the MD portion of the context.
	 */
	memset(&ucp->uc_mcontext, 0, sizeof(ucp->uc_mcontext));
	ucp->uc_mcontext.mc_special.sp = (intptr_t)args - 16;
	ucp->uc_mcontext.mc_special.bspstore = (intptr_t)bsp;
	ucp->uc_mcontext.mc_special.pfs = (3 << 7) | 3;
	ucp->uc_mcontext.mc_special.rsc = 0xf;
	ucp->uc_mcontext.mc_special.rp = ((struct fdesc*)ctx_wrapper)->ip;
	ucp->uc_mcontext.mc_special.gp = ((struct fdesc*)ctx_wrapper)->gp;
	ucp->uc_mcontext.mc_special.fpsr = IA64_FPSR_DEFAULT;
}