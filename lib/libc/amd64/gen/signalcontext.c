
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
#include <signal.h>
#include <stdlib.h>
#include <strings.h>

typedef void (*handler_t)(uint64_t, uint64_t, uint64_t);

/* Prototypes */
static void sigctx_wrapper(ucontext_t *ucp, handler_t func, uint64_t *args);

__weak_reference(__signalcontext, signalcontext);

int
__signalcontext(ucontext_t *ucp, int sig, __sighandler_t *func)
{
	uint64_t *args;
	siginfo_t *sig_si;
	ucontext_t *sig_uc;
	uint64_t sp;

	/* Bail out if we don't have a valid ucontext pointer. */
	if (ucp == NULL)
		abort();

	/*
	 * Build a signal frame and copy the arguments of signal handler
	 * 'func' onto the stack and do the funky stack alignment.
	 * This means that we need an 8-byte-odd alignment since the ABI expects
	 * the return address to be pushed, thus breaking the 16 byte alignment.
	 */
	sp = (ucp->uc_mcontext.mc_rsp - 128 - sizeof(ucontext_t)) & ~15UL;
	sig_uc = (ucontext_t *)sp;
	bcopy(ucp, sig_uc, sizeof(*sig_uc));
	sp = (sp - sizeof(siginfo_t)) & ~15UL;
	sig_si = (siginfo_t *)sp;
	bzero(sig_si, sizeof(*sig_si));
	sig_si->si_signo = sig;
	sp -= 3 * sizeof(uint64_t);
	args = (uint64_t *)sp;
	args[0] = sig;
	args[1] = (intptr_t)sig_si;
	args[2] = (intptr_t)sig_uc;
	sp -= 16;

	/*
	 * Setup the ucontext of the signal handler.
	 */
	bzero(&ucp->uc_mcontext, sizeof(ucp->uc_mcontext));
	ucp->uc_mcontext.mc_fpformat = _MC_FPFMT_NODEV;
	ucp->uc_mcontext.mc_ownedfp = _MC_FPOWNED_NONE;
	ucp->uc_link = sig_uc;
	sigdelset(&ucp->uc_sigmask, sig);

	ucp->uc_mcontext.mc_len = sizeof(mcontext_t);
	ucp->uc_mcontext.mc_rdi = (register_t)ucp;
	ucp->uc_mcontext.mc_rsi = (register_t)func;
	ucp->uc_mcontext.mc_rdx = (register_t)args;
	ucp->uc_mcontext.mc_rbp = (register_t)sp;
	ucp->uc_mcontext.mc_rbx = (register_t)sp;
	ucp->uc_mcontext.mc_rsp = (register_t)sp;
	ucp->uc_mcontext.mc_rip = (register_t)sigctx_wrapper;
	return (0);
}

static void
sigctx_wrapper(ucontext_t *ucp, handler_t func, uint64_t *args)
{

	(*func)(args[0], args[1], args[2]);
	if (ucp->uc_link == NULL)
		exit(0);
	setcontext((const ucontext_t *)ucp->uc_link);
	/* should never get here */
	abort();
	/* NOTREACHED */
}