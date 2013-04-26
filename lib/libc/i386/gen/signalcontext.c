
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
#include <machine/psl.h>
#include <machine/sigframe.h>
#include <signal.h>
#include <strings.h>

__weak_reference(__signalcontext, signalcontext);

extern void _ctx_start(ucontext_t *, int argc, ...);

int
__signalcontext(ucontext_t *ucp, int sig, __sighandler_t *func)
{
	register_t *p;
	struct sigframe *sfp;

	/*-
	 * Set up stack.
	 * (n = sizeof(int))
	 * 2n+sizeof(struct sigframe)	ucp
	 * 2n				struct sigframe
	 * 1n				&func
	 * 0n				&_ctx_start
	 */
	p = (register_t *)(void *)(intptr_t)ucp->uc_mcontext.mc_esp;
	*--p = (register_t)(intptr_t)ucp;
	p = (register_t *)((u_register_t)p & ~0xF);  /* Align to 16 bytes. */
	p = (register_t *)((u_register_t)p - sizeof(struct sigframe));
	sfp = (struct sigframe *)p;
	bzero(sfp, sizeof(struct sigframe));
	sfp->sf_signum = sig;
	sfp->sf_siginfo = (register_t)(intptr_t)&sfp->sf_si;
	sfp->sf_ucontext = (register_t)(intptr_t)&sfp->sf_uc;
	sfp->sf_ahu.sf_action = (__siginfohandler_t *)func;
	bcopy(ucp, &sfp->sf_uc, sizeof(ucontext_t));
	sfp->sf_si.si_signo = sig;
	*--p = (register_t)(intptr_t)func;

	/*
	 * Set up ucontext_t.
	 */
	ucp->uc_mcontext.mc_esi = ucp->uc_mcontext.mc_esp - sizeof(int);
	ucp->uc_mcontext.mc_esp = (register_t)(intptr_t)p;
	ucp->uc_mcontext.mc_eip = (register_t)(intptr_t)_ctx_start;
	ucp->uc_mcontext.mc_eflags &= ~PSL_T;
	ucp->uc_link = &sfp->sf_uc;
	sigdelset(&ucp->uc_sigmask, sig);
	return (0);
}