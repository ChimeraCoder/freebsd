
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
#include <sys/ucontext.h>
#include <signal.h>
#include <stdlib.h>
#include <strings.h>

typedef void (*handler_t)(uint32_t, uint32_t, uint32_t);

/* Prototypes */
static void ctx_wrapper(ucontext_t *ucp, handler_t func, uint32_t sig,
			uint32_t sig_si, uint32_t sig_uc);

__weak_reference(__signalcontext, signalcontext);

int
__signalcontext(ucontext_t *ucp, int sig, __sighandler_t *func)
{
	siginfo_t *sig_si;
	ucontext_t *sig_uc;
	uint32_t sp;

	/* Bail out if we don't have a valid ucontext pointer. */
	if (ucp == NULL)
		abort();

	/*
	 * Build a 16-byte-aligned signal frame
	 */
	sp = (ucp->uc_mcontext.mc_gpr[1] - sizeof(ucontext_t)) & ~15UL;
	sig_uc = (ucontext_t *)sp;
	bcopy(ucp, sig_uc, sizeof(*sig_uc));
	sp = (sp - sizeof(siginfo_t)) & ~15UL;
	sig_si = (siginfo_t *)sp;
	bzero(sig_si, sizeof(*sig_si));
	sig_si->si_signo = sig;

	/*
	 * Subtract 8 bytes from stack to allow for frameptr
	 */
	sp -= 2*sizeof(uint32_t);
	sp &= ~15UL;

	/*
	 * Setup the ucontext of the signal handler.
	 */
	bzero(&ucp->uc_mcontext, sizeof(ucp->uc_mcontext));
	ucp->uc_link = sig_uc;
	sigdelset(&ucp->uc_sigmask, sig);

	ucp->uc_mcontext.mc_vers = _MC_VERSION;
	ucp->uc_mcontext.mc_len = sizeof(struct __mcontext);
	ucp->uc_mcontext.mc_srr0 = (uint32_t) ctx_wrapper;
	ucp->uc_mcontext.mc_gpr[1] = (uint32_t) sp;
	ucp->uc_mcontext.mc_gpr[3] = (uint32_t) func;
	ucp->uc_mcontext.mc_gpr[4] = (uint32_t) sig;
	ucp->uc_mcontext.mc_gpr[5] = (uint32_t) sig_si;
	ucp->uc_mcontext.mc_gpr[6] = (uint32_t) sig_uc;

	return (0);
}

static void
ctx_wrapper(ucontext_t *ucp, handler_t func, uint32_t sig, uint32_t sig_si,
	    uint32_t sig_uc)
{

	(*func)(sig, sig_si, sig_uc);
	if (ucp->uc_link == NULL)
		exit(0);
	setcontext((const ucontext_t *)ucp->uc_link);
	/* should never get here */
	abort();
	/* NOTREACHED */
}