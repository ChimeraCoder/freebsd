
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
#include <sys/pmc.h>
#include <sys/proc.h>
#include <sys/systm.h>

#include <machine/cpu.h>
#include <machine/md_var.h>
#include <machine/pmc_mdep.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>

struct pmc_mdep *
pmc_md_initialize()
{
#ifdef CPU_XSCALE_IXP425
	if (cpu_class == CPU_CLASS_XSCALE)
		return pmc_xscale_initialize();
	else
#endif
		return NULL;
}

void
pmc_md_finalize(struct pmc_mdep *md)
{
#ifdef CPU_XSCALE_IXP425
	if (cpu_class == CPU_CLASS_XSCALE)
		pmc_xscale_finalize(md);
	else
		KASSERT(0, ("[arm,%d] Unknown CPU Class 0x%x", __LINE__,
		    cpu_class));
#endif
}

int
pmc_save_kernel_callchain(uintptr_t *cc, int maxsamples,
    struct trapframe *tf)
{
	uintptr_t pc, r, stackstart, stackend, fp;
	struct thread *td;
	int count;

	KASSERT(TRAPF_USERMODE(tf) == 0,("[arm,%d] not a kernel backtrace",
	    __LINE__));

	td = curthread;
	pc = PMC_TRAPFRAME_TO_PC(tf);
	*cc++ = pc;

	if (maxsamples <= 1)
		return (1);

	stackstart = (uintptr_t) td->td_kstack;
	stackend = (uintptr_t) td->td_kstack + td->td_kstack_pages * PAGE_SIZE;
	fp = PMC_TRAPFRAME_TO_FP(tf);

	if (!PMC_IN_KERNEL(pc) ||
	    !PMC_IN_KERNEL_STACK(fp, stackstart, stackend))
		return (1);

	for (count = 1; count < maxsamples; count++) {
		/* Use saved lr as pc. */
		r = fp - sizeof(uintptr_t);
		if (!PMC_IN_KERNEL_STACK(r, stackstart, stackend))
			break;
		pc = *(uintptr_t *)r;
		if (!PMC_IN_KERNEL(pc))
			break;

		*cc++ = pc;

		/* Switch to next frame up */
		r = fp - 3 * sizeof(uintptr_t);
		if (!PMC_IN_KERNEL_STACK(r, stackstart, stackend))
			break;
		fp = *(uintptr_t *)r;
		if (!PMC_IN_KERNEL_STACK(fp, stackstart, stackend))
			break;
	}

	return (count);
}

int
pmc_save_user_callchain(uintptr_t *cc, int maxsamples,
    struct trapframe *tf)
{
	uintptr_t pc, r, oldfp, fp;
	struct thread *td;
	int count;

	KASSERT(TRAPF_USERMODE(tf), ("[x86,%d] Not a user trap frame tf=%p",
	    __LINE__, (void *) tf));

	td = curthread;
	pc = PMC_TRAPFRAME_TO_PC(tf);
	*cc++ = pc;

	if (maxsamples <= 1)
		return (1);

	oldfp = fp = PMC_TRAPFRAME_TO_FP(tf);

	if (!PMC_IN_USERSPACE(pc) ||
	    !PMC_IN_USERSPACE(fp))
		return (1);

	for (count = 1; count < maxsamples; count++) {
		/* Use saved lr as pc. */
		r = fp - sizeof(uintptr_t);
		if (copyin((void *)r, &pc, sizeof(pc)) != 0)
			break;
		if (!PMC_IN_USERSPACE(pc))
			break;

		*cc++ = pc;

		/* Switch to next frame up */
		oldfp = fp;
		r = fp - 3 * sizeof(uintptr_t);
		if (copyin((void *)r, &fp, sizeof(fp)) != 0)
			break;
		if (fp < oldfp || !PMC_IN_USERSPACE(fp))
			break;
	}

	return (count);
}