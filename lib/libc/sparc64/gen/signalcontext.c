
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

#include <machine/frame.h>
#include <machine/sigframe.h>

#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

__weak_reference(__signalcontext, signalcontext);

extern void _ctx_start(void);

int
__signalcontext(ucontext_t *ucp, int sig, __sighandler_t *func)
{
	struct sigframe *sfp;
	struct frame *fp;
	mcontext_t *mc;

	mc = &ucp->uc_mcontext;
	sfp = (struct sigframe *)(mc->mc_sp + SPOFF) - 1;
	fp = (struct frame *)sfp - 1;

	bzero(fp, sizeof(*fp));

	bzero(sfp, sizeof(*sfp));
	bcopy(ucp, &sfp->sf_uc, sizeof(*ucp));
	sfp->sf_si.si_signo = sig;

	mc->mc_global[1] = (uint64_t)func;
	mc->mc_global[2] = (uint64_t)ucp;
	mc->mc_out[0] = sig;
	mc->mc_out[1] = (uint64_t)&sfp->sf_si;
	mc->mc_out[2] = (uint64_t)&sfp->sf_uc;
	mc->mc_out[6] = (uint64_t)fp - SPOFF;
	mc->mc_tnpc = (uint64_t)_ctx_start + 4;
	mc->mc_tpc = (uint64_t)_ctx_start;

	ucp->uc_link = &sfp->sf_uc;
	sigdelset(&ucp->uc_sigmask, sig);

	return (0);
}