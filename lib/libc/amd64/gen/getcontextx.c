
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
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <machine/cpufunc.h>
#include <machine/fpu.h>
#include <machine/specialreg.h>
#include <machine/sysarch.h>

static int xstate_sz = -1;

int
__getcontextx_size(void)
{
	u_int p[4];

	if (xstate_sz == -1) {
		do_cpuid(1, p);
		if ((p[2] & CPUID2_OSXSAVE) != 0) {
			cpuid_count(0xd, 0x0, p);
			xstate_sz = p[1] - sizeof(struct savefpu);
		} else
			xstate_sz = 0;
	}

	return (sizeof(ucontext_t) + xstate_sz);
}

int
__fillcontextx(char *ctx)
{
	struct amd64_get_xfpustate xfpu;
	ucontext_t *ucp;

	ucp = (ucontext_t *)ctx;
	if (getcontext(ucp) == -1)
		return (-1);
	if (xstate_sz != 0) {
		xfpu.addr = (char *)(ucp + 1);
		xfpu.len = xstate_sz;
		if (sysarch(AMD64_GET_XFPUSTATE, &xfpu) == -1)
			return (-1);
		ucp->uc_mcontext.mc_xfpustate = (__register_t)xfpu.addr;
		ucp->uc_mcontext.mc_xfpustate_len = xstate_sz;
		ucp->uc_mcontext.mc_flags |= _MC_HASFPXSTATE;
	} else {
		ucp->uc_mcontext.mc_xfpustate = 0;
		ucp->uc_mcontext.mc_xfpustate_len = 0;
	}
	return (0);
}

__weak_reference(__getcontextx, getcontextx);

ucontext_t *
__getcontextx(void)
{
	char *ctx;
	int error;

	ctx = malloc(__getcontextx_size());
	if (ctx == NULL)
		return (NULL);
	if (__fillcontextx(ctx) == -1) {
		error = errno;
		free(ctx);
		errno = error;
		return (NULL);
	}
	return ((ucontext_t *)ctx);
}