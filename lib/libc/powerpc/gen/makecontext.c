
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

#include <sys/param.h>

#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ucontext.h>

__weak_reference(__makecontext, makecontext);

void _ctx_done(ucontext_t *ucp);
void _ctx_start(void);

void
_ctx_done(ucontext_t *ucp)
{
	if (ucp->uc_link == NULL)
		exit(0);
	else {
		/* invalidate context */
		ucp->uc_mcontext.mc_len = 0;

		setcontext((const ucontext_t *)ucp->uc_link);

		abort(); /* should never return from above call */
	}
}

void
__makecontext(ucontext_t *ucp, void (*start)(void), int argc, ...)
{
	mcontext_t *mc;
	char *sp;
	va_list ap;
	int i, regargs, stackargs;

	/* Sanity checks */
	if ((ucp == NULL) || (argc < 0) || (argc > NCARGS)
	    || (ucp->uc_stack.ss_sp == NULL)
	    || (ucp->uc_stack.ss_size < MINSIGSTKSZ)) {
		/* invalidate context */
		ucp->uc_mcontext.mc_len = 0;
		return;
	}

	/*
	 * The stack must have space for the frame pointer, saved
	 * link register, overflow arguments, and be 16-byte
	 * aligned.
	 */
	stackargs = (argc > 8) ? argc - 8 : 0;
	sp = (char *) ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size
		- sizeof(uint32_t)*(stackargs + 2);
	sp = (char *)((uint32_t)sp & ~0x1f);

	mc = &ucp->uc_mcontext;

	/*
	 * Up to 8 register args. Assumes all args are 32-bit and
	 * integer only. Not sure how to cater for floating point,
	 * although 64-bit args will work if aligned correctly
	 * in the arg list.
	 */
	regargs = (argc > 8) ? 8 : argc;
	va_start(ap, argc);
	for (i = 0; i < regargs; i++)
		mc->mc_gpr[3 + i] = va_arg(ap, uint32_t);

	/*
	 * Overflow args go onto the stack
	 */
	if (argc > 8) {
		uint32_t *argp;

		/* Skip past frame pointer and saved LR */
		argp = (uint32_t *)sp + 2;

		for (i = 0; i < stackargs; i++)
			*argp++ = va_arg(ap, uint32_t);
	}
	va_end(ap);

	/*
	 * Use caller-saved regs 14/15 to hold params that _ctx_start
	 * will use to invoke the user-supplied func
	 */
	mc->mc_srr0 = (uint32_t) _ctx_start;
	mc->mc_gpr[1] = (uint32_t) sp;		/* new stack pointer */
	mc->mc_gpr[14] = (uint32_t) start;	/* r14 <- start */
	mc->mc_gpr[15] = (uint32_t) ucp;	/* r15 <- ucp */
}