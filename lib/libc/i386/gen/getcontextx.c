
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
#include <machine/npx.h>
#include <machine/specialreg.h>
#include <machine/sysarch.h>

static int xstate_sz = -1;

int
__getcontextx_size(void)
{
	u_int p[4];
	int cpuid_supported;

	if (xstate_sz == -1) {
		__asm __volatile(
		    "	pushfl\n"
		    "	popl	%%eax\n"
		    "	movl    %%eax,%%ecx\n"
		    "	xorl    $0x200000,%%eax\n"
		    "	pushl	%%eax\n"
		    "	popfl\n"
		    "	pushfl\n"
		    "	popl    %%eax\n"
		    "	xorl    %%eax,%%ecx\n"
		    "	je	1f\n"
		    "	movl	$1,%0\n"
		    "	jmp	2f\n"
		    "1:	movl	$0,%0\n"
		    "2:\n"
		    : "=r" (cpuid_supported) : : "eax", "ecx");
		if (cpuid_supported) {
			__asm __volatile(
			    "	pushl	%%ebx\n"
			    "	cpuid\n"
			    "	movl	%%ebx,%1\n"
			    "	popl	%%ebx\n"
			    : "=a" (p[0]), "=r" (p[1]), "=c" (p[2]), "=d" (p[3])
			    :  "0" (0x1));
			if ((p[2] & CPUID2_OSXSAVE) != 0) {
				__asm __volatile(
				    "	pushl	%%ebx\n"
				    "	cpuid\n"
				    "	movl	%%ebx,%1\n"
				    "	popl	%%ebx\n"
				    : "=a" (p[0]), "=r" (p[1]), "=c" (p[2]),
					"=d" (p[3])
				    :  "0" (0xd), "2" (0x0));
				xstate_sz = p[1] - sizeof(struct savexmm);
			} else
				xstate_sz = 0;
		} else
			xstate_sz = 0;
	}

	return (sizeof(ucontext_t) + xstate_sz);
}

int
__fillcontextx(char *ctx)
{
	struct i386_get_xfpustate xfpu;
	ucontext_t *ucp;

	ucp = (ucontext_t *)ctx;
	if (getcontext(ucp) == -1)
		return (-1);
	if (xstate_sz != 0) {
		xfpu.addr = (char *)(ucp + 1);
		xfpu.len = xstate_sz;
		if (sysarch(I386_GET_XFPUSTATE, &xfpu) == -1)
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