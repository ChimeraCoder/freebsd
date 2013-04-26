
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
#include <sys/proc.h>
#include <sys/stack.h>

#include <machine/pcb.h>
#include <machine/stack.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>

static void
stack_capture(struct stack *st, register_t ebp)
{
	struct i386_frame *frame;
	vm_offset_t callpc;

	stack_zero(st);
	frame = (struct i386_frame *)ebp;
	while (1) {
		if (!INKERNEL(frame))
			break;
		callpc = frame->f_retaddr;
		if (!INKERNEL(callpc))
			break;
		if (stack_put(st, callpc) == -1)
			break;
		if (frame->f_frame <= frame ||
		    (vm_offset_t)frame->f_frame >=
		    (vm_offset_t)ebp + KSTACK_PAGES * PAGE_SIZE)
			break;
		frame = frame->f_frame;
	}
}

void
stack_save_td(struct stack *st, struct thread *td)
{
	register_t ebp;

	if (TD_IS_SWAPPED(td))
		panic("stack_save_td: swapped");
	if (TD_IS_RUNNING(td))
		panic("stack_save_td: running");

	ebp = td->td_pcb->pcb_ebp;
	stack_capture(st, ebp);
}

void
stack_save(struct stack *st)
{
	register_t ebp;

	__asm __volatile("movl %%ebp,%0" : "=r" (ebp));
	stack_capture(st, ebp);
}