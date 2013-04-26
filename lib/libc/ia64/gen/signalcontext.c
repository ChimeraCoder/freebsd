
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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

struct fdesc {
	uint64_t ip;
	uint64_t gp;
};

typedef void (*handler_t)(uint64_t, uint64_t, uint64_t);

static __inline uint64_t *
spill(uint64_t *bsp, uint64_t arg)
{
	*bsp++ = arg;
	if (((intptr_t)bsp & 0x1ff) == 0x1f8)
		*bsp++ = 0;
	return (bsp);
}

static void
ctx_wrapper(ucontext_t *ucp, handler_t func, uint64_t *args)
{

	(*func)(args[0], args[1], args[2]);
	if (ucp->uc_link == NULL)
		exit(0);
	setcontext((const ucontext_t *)ucp->uc_link);
	/* should never get here */
	abort();
	/* NOTREACHED */
}

__weak_reference(__signalcontext, signalcontext);

int
__signalcontext(ucontext_t *ucp, int sig, __sighandler_t *func)
{
	uint64_t *args, *bsp;
	siginfo_t *sig_si;
	ucontext_t *sig_uc;
	uint64_t sp;

	/* Bail out if we don't have a valid ucontext pointer. */
	if (ucp == NULL)
		abort();

	/*
	 * Build a signal frame and copy the arguments of signal handler
	 * 'func' onto the (memory) stack. We only need 3 arguments, but
	 * we create room for 4 so that we are 16-byte aligned.
	 */
	sp = (ucp->uc_mcontext.mc_special.sp - sizeof(ucontext_t)) & ~15UL;
	sig_uc = (ucontext_t*)sp;
	bcopy(ucp, sig_uc, sizeof(*sig_uc));
	sp = (sp - sizeof(siginfo_t)) & ~15UL;
	sig_si = (siginfo_t*)sp;
	bzero(sig_si, sizeof(*sig_si));
	sig_si->si_signo = sig;
	sp -= 4 * sizeof(uint64_t);
	args = (uint64_t*)sp;
	args[0] = sig;
	args[1] = (intptr_t)sig_si;
	args[2] = (intptr_t)sig_uc;

	/*
	 * Push (spill) the arguments of the context wrapper onto the register
	 * stack. They get loaded by the RSE on a context switch.
	 */
	bsp = (uint64_t*)ucp->uc_mcontext.mc_special.bspstore;
	bsp = spill(bsp, (intptr_t)ucp);
	bsp = spill(bsp, (intptr_t)func);
	bsp = spill(bsp, (intptr_t)args);

	/*
	 * Setup the ucontext of the signal handler.
	 */
	memset(&ucp->uc_mcontext, 0, sizeof(ucp->uc_mcontext));
	ucp->uc_link = sig_uc;
	sigdelset(&ucp->uc_sigmask, sig);
	ucp->uc_mcontext.mc_special.sp = (intptr_t)args - 16;
	ucp->uc_mcontext.mc_special.bspstore = (intptr_t)bsp;
	ucp->uc_mcontext.mc_special.pfs = (3 << 7) | 3;
	ucp->uc_mcontext.mc_special.rsc = 0xf;
	ucp->uc_mcontext.mc_special.rp = ((struct fdesc*)ctx_wrapper)->ip;
	ucp->uc_mcontext.mc_special.gp = ((struct fdesc*)ctx_wrapper)->gp;
	ucp->uc_mcontext.mc_special.fpsr = IA64_FPSR_DEFAULT;
	return (0);
}