
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
#include <sys/stack.h>
#include <sys/systm.h>

#include <machine/pcb.h>
#include <machine/stack.h>
#include <machine/vmparam.h>

static void stack_capture(struct stack *st, struct frame *frame);

static void
stack_capture(struct stack *st, struct frame *frame)
{
	struct frame *fp;
	vm_offset_t callpc;

	stack_zero(st);
	fp = frame;
	for (;;) {
		if (!INKERNEL((vm_offset_t)fp) ||
		    !ALIGNED_POINTER(fp, uint64_t))
                        break;
		callpc = fp->fr_pc;
		if (!INKERNEL(callpc))
			break;
		/* Don't bother traversing trap frames. */
		if ((callpc > (uint64_t)tl_trap_begin &&
		    callpc < (uint64_t)tl_trap_end) ||
		    (callpc > (uint64_t)tl_text_begin &&
		    callpc < (uint64_t)tl_text_end))
			break;
		if (stack_put(st, callpc) == -1)
			break;
		if (v9next_frame(fp) <= fp ||
		    v9next_frame(fp) >= frame + KSTACK_PAGES * PAGE_SIZE)
			break;
		fp = v9next_frame(fp);
	}
}

void
stack_save_td(struct stack *st, struct thread *td)
{

	if (TD_IS_SWAPPED(td))
		panic("stack_save_td: swapped");
	if (TD_IS_RUNNING(td))
		panic("stack_save_td: running");

	stack_capture(st, (struct frame *)(td->td_pcb->pcb_sp + SPOFF));
}

void
stack_save(struct stack *st)
{

	stack_capture(st, (struct frame *)__builtin_frame_address(1));
}