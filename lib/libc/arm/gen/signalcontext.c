
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
#include <sys/signal.h>
#include <sys/ucontext.h>

#include <machine/frame.h>
#include <machine/sigframe.h>

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <signal.h>

__weak_reference(__signalcontext, signalcontext);

extern void _ctx_start(void);

int
__signalcontext(ucontext_t *ucp, int sig, __sighandler_t *func)
{
	struct sigframe *sfp;
	__greg_t *gr = ucp->uc_mcontext.__gregs;
	unsigned int *sp;

	sp = (unsigned int *)gr[_REG_SP];

	sfp = (struct sigframe *)sp - 1;
	
	bzero(sfp, sizeof(*sfp));
	bcopy(ucp, &sfp->sf_uc, sizeof(*ucp));
	sfp->sf_si.si_signo = sig;

	gr[_REG_SP] = (__greg_t)sfp;
	/* Wipe out frame pointer. */
	gr[_REG_FP] = 0;
	/* Arrange for return via the trampoline code. */
	gr[_REG_PC] = (__greg_t)_ctx_start;
	gr[_REG_R4] = (__greg_t)func;
	gr[_REG_R5] = (__greg_t)ucp;
	gr[_REG_R0] = (__greg_t)sig;
	gr[_REG_R1] = (__greg_t)&sfp->sf_si;
	gr[_REG_R2] = (__greg_t)&sfp->sf_uc;

	ucp->uc_link = &sfp->sf_uc;
	sigdelset(&ucp->uc_sigmask, sig);

	return (0);
}