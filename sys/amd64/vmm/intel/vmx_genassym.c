
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
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/proc.h>
#include <sys/assym.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/pmap.h>

#include <machine/vmm.h>
#include "vmx.h"
#include "vmx_cpufunc.h"

ASSYM(VMXCTX_TMPSTKTOP, offsetof(struct vmxctx, tmpstktop));
ASSYM(VMXCTX_GUEST_RDI, offsetof(struct vmxctx, guest_rdi));
ASSYM(VMXCTX_GUEST_RSI, offsetof(struct vmxctx, guest_rsi));
ASSYM(VMXCTX_GUEST_RDX, offsetof(struct vmxctx, guest_rdx));
ASSYM(VMXCTX_GUEST_RCX, offsetof(struct vmxctx, guest_rcx));
ASSYM(VMXCTX_GUEST_R8, offsetof(struct vmxctx, guest_r8));
ASSYM(VMXCTX_GUEST_R9, offsetof(struct vmxctx, guest_r9));
ASSYM(VMXCTX_GUEST_RAX, offsetof(struct vmxctx, guest_rax));
ASSYM(VMXCTX_GUEST_RBX, offsetof(struct vmxctx, guest_rbx));
ASSYM(VMXCTX_GUEST_RBP, offsetof(struct vmxctx, guest_rbp));
ASSYM(VMXCTX_GUEST_R10, offsetof(struct vmxctx, guest_r10));
ASSYM(VMXCTX_GUEST_R11, offsetof(struct vmxctx, guest_r11));
ASSYM(VMXCTX_GUEST_R12, offsetof(struct vmxctx, guest_r12));
ASSYM(VMXCTX_GUEST_R13, offsetof(struct vmxctx, guest_r13));
ASSYM(VMXCTX_GUEST_R14, offsetof(struct vmxctx, guest_r14));
ASSYM(VMXCTX_GUEST_R15, offsetof(struct vmxctx, guest_r15));
ASSYM(VMXCTX_GUEST_CR2, offsetof(struct vmxctx, guest_cr2));

ASSYM(VMXCTX_HOST_R15, offsetof(struct vmxctx, host_r15));
ASSYM(VMXCTX_HOST_R14, offsetof(struct vmxctx, host_r14));
ASSYM(VMXCTX_HOST_R13, offsetof(struct vmxctx, host_r13));
ASSYM(VMXCTX_HOST_R12, offsetof(struct vmxctx, host_r12));
ASSYM(VMXCTX_HOST_RBP, offsetof(struct vmxctx, host_rbp));
ASSYM(VMXCTX_HOST_RSP, offsetof(struct vmxctx, host_rsp));
ASSYM(VMXCTX_HOST_RBX, offsetof(struct vmxctx, host_rbx));
ASSYM(VMXCTX_HOST_RIP, offsetof(struct vmxctx, host_rip));

ASSYM(VMXCTX_LAUNCH_ERROR, offsetof(struct vmxctx, launch_error));

ASSYM(VM_SUCCESS,	VM_SUCCESS);
ASSYM(VM_FAIL_INVALID,	VM_FAIL_INVALID);
ASSYM(VM_FAIL_VALID,	VM_FAIL_VALID);

ASSYM(VMX_RETURN_DIRECT,	VMX_RETURN_DIRECT);
ASSYM(VMX_RETURN_LONGJMP,	VMX_RETURN_LONGJMP);
ASSYM(VMX_RETURN_VMRESUME,	VMX_RETURN_VMRESUME);
ASSYM(VMX_RETURN_VMLAUNCH,	VMX_RETURN_VMLAUNCH);
ASSYM(VMX_RETURN_AST,		VMX_RETURN_AST);

ASSYM(TDF_ASTPENDING, TDF_ASTPENDING);
ASSYM(TDF_NEEDRESCHED, TDF_NEEDRESCHED);
ASSYM(TD_FLAGS, offsetof(struct thread, td_flags));
ASSYM(PC_CURTHREAD, offsetof(struct pcpu, pc_curthread));