
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

/*
 * 386 Trap and System call handling
 */

#include "opt_clock.h"
#include "opt_compat.h"
#include "opt_cpu.h"
#include "opt_isa.h"

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/pioctl.h>
#include <sys/kernel.h>
#include <sys/ktr.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/ptrace.h>
#include <sys/resourcevar.h>
#include <sys/signalvar.h>
#include <sys/syscall.h>
#include <sys/sysctl.h>
#include <sys/sysent.h>
#include <sys/uio.h>
#include <sys/vmmeter.h>
#include <security/audit/audit.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>
#include <vm/vm_kern.h>
#include <vm/vm_map.h>
#include <vm/vm_page.h>
#include <vm/vm_extern.h>

#include <machine/cpu.h>
#include <machine/intr_machdep.h>
#include <machine/md_var.h>

#include <compat/freebsd32/freebsd32_signal.h>
#include <compat/freebsd32/freebsd32_util.h>
#include <compat/ia32/ia32_signal.h>
#include <machine/psl.h>
#include <machine/segments.h>
#include <machine/specialreg.h>
#include <machine/sysarch.h>
#include <machine/frame.h>
#include <machine/md_var.h>
#include <machine/pcb.h>
#include <machine/cpufunc.h>

#define	IDTVEC(name)	__CONCAT(X,name)

extern inthand_t IDTVEC(int0x80_syscall), IDTVEC(rsvd);

void ia32_syscall(struct trapframe *frame);	/* Called from asm code */

void
ia32_set_syscall_retval(struct thread *td, int error)
{

	cpu_set_syscall_retval(td, error);
}

int
ia32_fetch_syscall_args(struct thread *td, struct syscall_args *sa)
{
	struct proc *p;
	struct trapframe *frame;
	caddr_t params;
	u_int32_t args[8];
	int error, i;

	p = td->td_proc;
	frame = td->td_frame;

	params = (caddr_t)frame->tf_rsp + sizeof(u_int32_t);
	sa->code = frame->tf_rax;

	/*
	 * Need to check if this is a 32 bit or 64 bit syscall.
	 */
	if (sa->code == SYS_syscall) {
		/*
		 * Code is first argument, followed by actual args.
		 */
		sa->code = fuword32(params);
		params += sizeof(int);
	} else if (sa->code == SYS___syscall) {
		/*
		 * Like syscall, but code is a quad, so as to maintain
		 * quad alignment for the rest of the arguments.
		 * We use a 32-bit fetch in case params is not
		 * aligned.
		 */
		sa->code = fuword32(params);
		params += sizeof(quad_t);
	}
 	if (p->p_sysent->sv_mask)
 		sa->code &= p->p_sysent->sv_mask;
 	if (sa->code >= p->p_sysent->sv_size)
 		sa->callp = &p->p_sysent->sv_table[0];
  	else
 		sa->callp = &p->p_sysent->sv_table[sa->code];
	sa->narg = sa->callp->sy_narg;

	if (params != NULL && sa->narg != 0)
		error = copyin(params, (caddr_t)args,
		    (u_int)(sa->narg * sizeof(int)));
	else
		error = 0;

	for (i = 0; i < sa->narg; i++)
		sa->args[i] = args[i];

	if (error == 0) {
		td->td_retval[0] = 0;
		td->td_retval[1] = frame->tf_rdx;
	}

	return (error);
}

#include "../../kern/subr_syscall.c"

void
ia32_syscall(struct trapframe *frame)
{
	struct thread *td;
	struct syscall_args sa;
	register_t orig_tf_rflags;
	int error;
	ksiginfo_t ksi;

	orig_tf_rflags = frame->tf_rflags;
	td = curthread;
	td->td_frame = frame;

	error = syscallenter(td, &sa);

	/*
	 * Traced syscall.
	 */
	if (orig_tf_rflags & PSL_T) {
		frame->tf_rflags &= ~PSL_T;
		ksiginfo_init_trap(&ksi);
		ksi.ksi_signo = SIGTRAP;
		ksi.ksi_code = TRAP_TRACE;
		ksi.ksi_addr = (void *)frame->tf_rip;
		trapsignal(td, &ksi);
	}

	syscallret(td, error, &sa);
}

static void
ia32_syscall_enable(void *dummy)
{

 	setidt(IDT_SYSCALL, &IDTVEC(int0x80_syscall), SDT_SYSIGT, SEL_UPL, 0);
}

static void
ia32_syscall_disable(void *dummy)
{

 	setidt(IDT_SYSCALL, &IDTVEC(rsvd), SDT_SYSIGT, SEL_KPL, 0);
}

SYSINIT(ia32_syscall, SI_SUB_EXEC, SI_ORDER_ANY, ia32_syscall_enable, NULL);
SYSUNINIT(ia32_syscall, SI_SUB_EXEC, SI_ORDER_ANY, ia32_syscall_disable, NULL);

#ifdef COMPAT_43
int
setup_lcall_gate(void)
{
	struct i386_ldt_args uap;
	struct user_segment_descriptor descs[2];
	struct gate_descriptor *ssd;
	uint32_t lcall_addr;
	int error;

	bzero(&uap, sizeof(uap));
	uap.start = 0;
	uap.num = 2;

	/*
	 * This is the easiest way to cut the space for system
	 * descriptor in ldt.  Manually adjust the descriptor type to
	 * the call gate later.
	 */
	bzero(&descs[0], sizeof(descs));
	descs[0].sd_type = SDT_SYSNULL;
	descs[1].sd_type = SDT_SYSNULL;
	error = amd64_set_ldt(curthread, &uap, descs);
	if (error != 0)
		return (error);

	lcall_addr = curproc->p_sysent->sv_psstrings - sz_lcall_tramp;
	mtx_lock(&dt_lock);
	ssd = (struct gate_descriptor *)(curproc->p_md.md_ldt->ldt_base);
	bzero(ssd, sizeof(*ssd));
	ssd->gd_looffset = lcall_addr;
	ssd->gd_hioffset = lcall_addr >> 16;
	ssd->gd_selector = _ucodesel;
	ssd->gd_type = SDT_SYSCGT;
	ssd->gd_dpl = SEL_UPL;
	ssd->gd_p = 1;
	mtx_unlock(&dt_lock);

	return (0);
}
#endif