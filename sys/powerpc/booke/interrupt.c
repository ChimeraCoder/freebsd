
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
__FBSDID("$FreeBSD$");

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
#include <machine/db_machdep.h>
#include <machine/frame.h>
#include <machine/intr_machdep.h>
#include <machine/md_var.h>
#include <machine/pcb.h>
#include <machine/psl.h>
#include <machine/trap.h>

#include "pic_if.h"

extern void decr_intr(struct trapframe *);

void powerpc_decr_interrupt(struct trapframe *);
void powerpc_extr_interrupt(struct trapframe *);
void powerpc_crit_interrupt(struct trapframe *);
void powerpc_mchk_interrupt(struct trapframe *);

static void dump_frame(struct trapframe *framep);

static void
dump_frame(struct trapframe *frame)
{
	int i;

	printf("\n*** *** STACK FRAME DUMP *** ***\n");
	printf("  exc  = 0x%x\n", frame->exc);
	printf("  srr0 = 0x%08x\n", frame->srr0);
	printf("  srr1 = 0x%08x\n", frame->srr1);
	printf("  dear = 0x%08x\n", frame->cpu.booke.dear);
	printf("  esr  = 0x%08x\n", frame->cpu.booke.esr);
	printf("  lr   = 0x%08x\n", frame->lr);
	printf("  cr   = 0x%08x\n", frame->cr);
	printf("  sp   = 0x%08x\n", frame->fixreg[1]);

	for (i = 0; i < 32; i++) {
		printf("  R%02d = 0x%08x", i, frame->fixreg[i]);
		if ((i & 0x3) == 3)
			printf("\n");
	}
	printf("\n");
}

void powerpc_crit_interrupt(struct trapframe *framep)
{

	printf("powerpc_crit_interrupt: critical interrupt!\n");
	dump_frame(framep);
	trap(framep);
}

void powerpc_mchk_interrupt(struct trapframe *framep)
{

	printf("powerpc_mchk_interrupt: machine check interrupt!\n");
	dump_frame(framep);
	trap(framep);
}

/*
 * Decrementer interrupt routine
 */
void
powerpc_decr_interrupt(struct trapframe *framep)
{
	struct thread *td;
	struct trapframe *oldframe;

	td = curthread;
	critical_enter();
	atomic_add_int(&td->td_intr_nesting_level, 1);
	oldframe = td->td_intr_frame;
	td->td_intr_frame = framep;
	decr_intr(framep);
	td->td_intr_frame = oldframe;
	atomic_subtract_int(&td->td_intr_nesting_level, 1);
	critical_exit();
	framep->srr1 &= ~PSL_WE;
}

/*
 * External input interrupt routine
 */
void
powerpc_extr_interrupt(struct trapframe *framep)
{

	critical_enter();
	PIC_DISPATCH(root_pic, framep);
	critical_exit();
	framep->srr1 &= ~PSL_WE;
}