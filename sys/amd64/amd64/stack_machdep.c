
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
stack_capture(struct stack *st, register_t rbp)
{
	struct amd64_frame *frame;
	vm_offset_t callpc;

	stack_zero(st);
	frame = (struct amd64_frame *)rbp;
	while (1) {
		if (!INKERNEL((long)frame))
			break;
		callpc = frame->f_retaddr;
		if (!INKERNEL(callpc))
			break;
		if (stack_put(st, callpc) == -1)
			break;
		if (frame->f_frame <= frame ||
		    (vm_offset_t)frame->f_frame >=
		    (vm_offset_t)rbp + KSTACK_PAGES * PAGE_SIZE)
			break;
		frame = frame->f_frame;
	}
}

void
stack_save_td(struct stack *st, struct thread *td)
{
	register_t rbp;

	if (TD_IS_SWAPPED(td))
		panic("stack_save_td: swapped");
	if (TD_IS_RUNNING(td))
		panic("stack_save_td: running");

	rbp = td->td_pcb->pcb_rbp;
	stack_capture(st, rbp);
}

void
stack_save(struct stack *st)
{
	register_t rbp;

	__asm __volatile("movq %%rbp,%0" : "=r" (rbp));
	stack_capture(st, rbp);
}