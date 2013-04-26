
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

/*
 * Interrupts are dispatched to here from locore asm
 */

#include <sys/cdefs.h>                  /* RCS ID & Copyright macro defns */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/interrupt.h>
#include <sys/kernel.h>
#include <sys/kthread.h>
#include <sys/ktr.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/smp.h>
#include <sys/unistd.h>
#include <sys/vmmeter.h>

#include <machine/cpu.h>
#include <machine/clock.h>
#include <machine/db_machdep.h>
#include <machine/fpu.h>
#include <machine/frame.h>
#include <machine/intr_machdep.h>
#include <machine/md_var.h>
#include <machine/pcb.h>
#include <machine/psl.h>
#include <machine/trap.h>
#include <machine/spr.h>
#include <machine/sr.h>

#include "pic_if.h"

/*
 * A very short dispatch, to try and maximise assembler code use
 * between all exception types. Maybe 'true' interrupts should go
 * here, and the trap code can come in separately
 */
void
powerpc_interrupt(struct trapframe *framep)
{
	struct thread *td;
	struct trapframe *oldframe;
	register_t ee;

	td = curthread;

	CTR2(KTR_INTR, "%s: EXC=%x", __func__, framep->exc);

	switch (framep->exc) {
	case EXC_EXI:
		critical_enter();
		PIC_DISPATCH(root_pic, framep);
		critical_exit();
		break;

	case EXC_DECR:
		critical_enter();
		atomic_add_int(&td->td_intr_nesting_level, 1);
		oldframe = td->td_intr_frame;
		td->td_intr_frame = framep;
		decr_intr(framep);
		td->td_intr_frame = oldframe;
		atomic_subtract_int(&td->td_intr_nesting_level, 1);
		critical_exit();
		break;

	default:
		/* Re-enable interrupts if applicable. */
		ee = framep->srr1 & PSL_EE;
		if (ee != 0)
			mtmsr(mfmsr() | ee);
		trap(framep);
	}	        
}