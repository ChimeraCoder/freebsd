
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
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/exec.h>
#include <sys/imgact.h>
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/sysent.h>
#include <sys/signalvar.h>
#include <sys/proc.h>
#include <sys/sx.h>

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_param.h>

#include <i386/ibcs2/ibcs2_syscall.h>
#include <i386/ibcs2/ibcs2_signal.h>

MODULE_VERSION(ibcs2, 1);

extern int bsd_to_ibcs2_errno[];
extern struct sysent ibcs2_sysent[IBCS2_SYS_MAXSYSCALL];
extern int szsigcode;
extern char sigcode[];
static int ibcs2_fixup(register_t **, struct image_params *);

struct sysentvec ibcs2_svr3_sysvec = {
        .sv_size	= sizeof (ibcs2_sysent) / sizeof (ibcs2_sysent[0]),
        .sv_table	= ibcs2_sysent,
        .sv_mask	= 0xff,
        .sv_sigsize	= IBCS2_SIGTBLSZ,
        .sv_sigtbl	= bsd_to_ibcs2_sig,
        .sv_errsize	= ELAST + 1,
        .sv_errtbl	= bsd_to_ibcs2_errno,
	.sv_transtrap	= NULL,
	.sv_fixup	= ibcs2_fixup,
	.sv_sendsig	= sendsig,
	.sv_sigcode	= sigcode,	/* use generic trampoline */
	.sv_szsigcode	= &szsigcode,
	.sv_prepsyscall	= NULL,
	.sv_name	= "IBCS2 COFF",
	.sv_coredump	= NULL,	/* we don't have a COFF coredump function */
	.sv_imgact_try	= NULL,
	.sv_minsigstksz	= IBCS2_MINSIGSTKSZ,
	.sv_pagesize	= PAGE_SIZE,
	.sv_minuser	= VM_MIN_ADDRESS,
	.sv_maxuser	= VM_MAXUSER_ADDRESS,
	.sv_usrstack	= USRSTACK,
	.sv_psstrings	= PS_STRINGS,
	.sv_stackprot	= VM_PROT_ALL,
	.sv_copyout_strings = exec_copyout_strings,
	.sv_setregs	= exec_setregs,
	.sv_fixlimit	= NULL,
	.sv_maxssiz	= NULL,
	.sv_flags	= SV_ABI_UNDEF | SV_IA32 | SV_ILP32,
	.sv_set_syscall_retval = cpu_set_syscall_retval,
	.sv_fetch_syscall_args = cpu_fetch_syscall_args,
	.sv_syscallnames = NULL,
	.sv_schedtail	= NULL,
};

static int
ibcs2_fixup(register_t **stack_base, struct image_params *imgp)
{

	return (suword(--(*stack_base), imgp->args->argc));
}

/*
 * Create an "ibcs2" module that does nothing but allow checking for
 * the presence of the subsystem.
 */
static int
ibcs2_modevent(module_t mod, int type, void *unused)
{
	struct proc *p = NULL;
	int rval = 0;

	switch(type) {
	case MOD_LOAD:
		break;
	case MOD_UNLOAD:
		/* if this was an ELF module we'd use elf_brand_inuse()... */
		sx_slock(&allproc_lock);
		FOREACH_PROC_IN_SYSTEM(p) {
			if (p->p_sysent == &ibcs2_svr3_sysvec) {
				rval = EBUSY;
				break;
			}
		}
		sx_sunlock(&allproc_lock);
		break;
	default:
	        rval = EOPNOTSUPP;
		break;
	}
	return (rval);
}
static moduledata_t ibcs2_mod = {
	"ibcs2",
	ibcs2_modevent,
	0
};
DECLARE_MODULE_TIED(ibcs2, ibcs2_mod, SI_SUB_PSEUDO, SI_ORDER_ANY);