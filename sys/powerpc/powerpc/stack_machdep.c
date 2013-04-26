
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

#include <vm/vm.h>
#include <vm/pmap.h>
#include <vm/vm_extern.h>

#include <machine/db_machdep.h>
#include <machine/pcb.h>
#include <machine/spr.h>
#include <machine/stack.h>
#include <machine/trap.h>

#ifdef __powerpc64__
#define CALLOFFSET 8 /* Account for the TOC reload slot */
#else
#define CALLOFFSET 4
#endif

static void
stack_capture(struct stack *st, vm_offset_t frame)
{
	vm_offset_t callpc;

	stack_zero(st);
	if (frame < PAGE_SIZE)
		return;
	while (1) {
		frame = *(vm_offset_t *)frame;
		if (frame < PAGE_SIZE)
			break;

	    #ifdef __powerpc64__
		callpc = *(vm_offset_t *)(frame + 16) - 4;
	    #else
		callpc = *(vm_offset_t *)(frame + 4) - 4;
	    #endif
		if ((callpc & 3) || (callpc < 0x100))
			break;

		/*
		 * Don't bother traversing trap-frames - there should
		 * be enough info down to the frame to work out where
		 * things are going wrong. Plus, prevents this shortened
		 * version of code from accessing user-space frames
		 */
		if (callpc + CALLOFFSET == (vm_offset_t) &trapexit ||
		    callpc + CALLOFFSET == (vm_offset_t) &asttrapexit)
			break;

		if (stack_put(st, callpc) == -1)
			break;
	}
}

void
stack_save_td(struct stack *st, struct thread *td)
{
	vm_offset_t frame;

	if (TD_IS_SWAPPED(td))
		panic("stack_save_td: swapped");
	if (TD_IS_RUNNING(td))
		panic("stack_save_td: running");

	frame = td->td_pcb->pcb_sp;
	stack_capture(st, frame);
}

void
stack_save(struct stack *st)
{
	register_t frame;

	frame = (register_t)__builtin_frame_address(1);
	stack_capture(st, frame);
}