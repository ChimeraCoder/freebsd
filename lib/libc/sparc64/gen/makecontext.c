
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
#include <machine/tstate.h>

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

__weak_reference(__makecontext, makecontext);

void _ctx_done(ucontext_t *ucp);
void _ctx_start(void);

void
__makecontext(ucontext_t *ucp, void (*start)(void), int argc, ...)
{
	mcontext_t *mc;
	uint64_t sp;
	va_list ap;
	int i;

	mc = &ucp->uc_mcontext;
	if (ucp == NULL ||
	    (mc->mc_flags & ((1L << _MC_VERSION_BITS) - 1)) != _MC_VERSION)
		return;
	if ((argc < 0) || (argc > 6) ||
	    (ucp->uc_stack.ss_sp == NULL) ||
	    (ucp->uc_stack.ss_size < MINSIGSTKSZ)) {
		mc->mc_flags = 0;
		return;
	}
	mc = &ucp->uc_mcontext;
	sp = (uint64_t)ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size;
	va_start(ap, argc);
	for (i = 0; i < argc; i++)
		mc->mc_out[i] = va_arg(ap, uint64_t);
	va_end(ap);
	mc->mc_global[1] = (uint64_t)start;
	mc->mc_global[2] = (uint64_t)ucp;
	mc->mc_out[6] = sp - SPOFF - sizeof(struct frame);
	mc->mc_tnpc = (uint64_t)_ctx_start + 4;
	mc->mc_tpc = (uint64_t)_ctx_start;
}

void
_ctx_done(ucontext_t *ucp)
{

	if (ucp->uc_link == NULL)
		exit(0);
	else {
		ucp->uc_mcontext.mc_flags = 0;
		setcontext((const ucontext_t *)ucp->uc_link);
		abort();
	}
}