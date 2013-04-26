
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

#include "opt_cpu.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <machine/md_var.h>
#include <machine/pcb.h>

#if !defined(CPU_DISABLE_SSE) && defined(I686_CPU)
#define CPU_ENABLE_SSE
#endif

int
cpu_ptrace(struct thread *td, int req, void *addr, int data)
{
#ifdef CPU_ENABLE_SSE
	struct savexmm *fpstate;
	int error;

	if (!cpu_fxsr)
		return (EINVAL);

	fpstate = &td->td_pcb->pcb_user_save.sv_xmm;
	switch (req) {
	case PT_GETXMMREGS:
		npxgetregs(td);
		error = copyout(fpstate, addr, sizeof(*fpstate));
		break;

	case PT_SETXMMREGS:
		npxgetregs(td);
		error = copyin(addr, fpstate, sizeof(*fpstate));
		fpstate->sv_env.en_mxcsr &= cpu_mxcsr_mask;
		break;

	default:
		return (EINVAL);
	}

	return (error);
#else
	return (EINVAL);
#endif
}