
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
#include <sys/proc.h>
#include <sys/systm.h>
#include <sys/limits.h>

#include <machine/fpu.h>
#include <machine/pcb.h>
#include <machine/psl.h>

void
enable_fpu(struct thread *td)
{
	int	msr;
	struct	pcb *pcb;
	struct	trapframe *tf;

	pcb = td->td_pcb;
	tf = trapframe(td);

	/*
	 * Save the thread's FPU CPU number, and set the CPU's current
	 * FPU thread
	 */
	td->td_pcb->pcb_fpcpu = PCPU_GET(cpuid);
	PCPU_SET(fputhread, td);

	/*
	 * Enable the FPU for when the thread returns from the exception.
	 * If this is the first time the FPU has been used by the thread,
	 * initialise the FPU registers and FPSCR to 0, and set the flag
	 * to indicate that the FPU is in use.
	 */
	tf->srr1 |= PSL_FP;
	if (!(pcb->pcb_flags & PCB_FPU)) {
		memset(&pcb->pcb_fpu, 0, sizeof pcb->pcb_fpu);
		pcb->pcb_flags |= PCB_FPU;
	}

	/*
	 * Temporarily enable floating-point so the registers
	 * can be restored.
	 */
	msr = mfmsr();
	mtmsr(msr | PSL_FP);
	isync();

	/*
	 * Load the floating point registers and FPSCR from the PCB.
	 * (A value of 0xff for mtfsf specifies that all 8 4-bit fields
	 * of the saved FPSCR are to be loaded from the FPU reg).
	 */
	__asm __volatile ("lfd 0,0(%0); mtfsf 0xff,0"
			  :: "b"(&pcb->pcb_fpu.fpscr));

#define LFP(n)   __asm ("lfd " #n ", 0(%0)" \
		:: "b"(&pcb->pcb_fpu.fpr[n]));
	LFP(0);		LFP(1);		LFP(2);		LFP(3);
	LFP(4);		LFP(5);		LFP(6);		LFP(7);
	LFP(8);		LFP(9);		LFP(10);	LFP(11);
	LFP(12);	LFP(13);	LFP(14);	LFP(15);
	LFP(16);	LFP(17);	LFP(18);	LFP(19);
	LFP(20);	LFP(21);	LFP(22);	LFP(23);
	LFP(24);	LFP(25);	LFP(26);	LFP(27);
	LFP(28);	LFP(29);	LFP(30);	LFP(31);
#undef LFP

	isync();
	mtmsr(msr);
}

void
save_fpu(struct thread *td)
{
	int	msr;
	struct	pcb *pcb;

	pcb = td->td_pcb;

	/*
	 * Temporarily re-enable floating-point during the save
	 */
	msr = mfmsr();
	mtmsr(msr | PSL_FP);
	isync();

	/*
	 * Save the floating-point registers and FPSCR to the PCB
	 */
#define SFP(n)   __asm ("stfd " #n ", 0(%0)" \
		:: "b"(&pcb->pcb_fpu.fpr[n]));
	SFP(0);		SFP(1);		SFP(2);		SFP(3);
	SFP(4);		SFP(5);		SFP(6);		SFP(7);
	SFP(8);		SFP(9);		SFP(10);	SFP(11);
	SFP(12);	SFP(13);	SFP(14);	SFP(15);
	SFP(16);	SFP(17);	SFP(18);	SFP(19);
	SFP(20);	SFP(21);	SFP(22);	SFP(23);
	SFP(24);	SFP(25);	SFP(26);	SFP(27);
	SFP(28);	SFP(29);	SFP(30);	SFP(31);
#undef SFP
	__asm __volatile ("mffs 0; stfd 0,0(%0)" :: "b"(&pcb->pcb_fpu.fpscr));

	/*
	 * Disable floating-point again
	 */
	isync();
	mtmsr(msr);

	/*
	 * Clear the current fp thread and pcb's CPU id
	 * XXX should this be left clear to allow lazy save/restore ?
	 */
	pcb->pcb_fpcpu = INT_MAX;
	PCPU_SET(fputhread, NULL);
}