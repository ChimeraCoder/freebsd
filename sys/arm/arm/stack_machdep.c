
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

#include <sys/systm.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/stack.h>

#include <machine/vmparam.h>
#include <machine/pcb.h>
#include <machine/stack.h>

static void
stack_capture(struct stack *st, u_int32_t *frame)
{
	vm_offset_t callpc;

	stack_zero(st);
	while (1) {
		if (!INKERNEL(frame))
			break;
		callpc = frame[FR_SCP];
		if (stack_put(st, callpc) == -1)
			break;
		frame = (u_int32_t *)(frame[FR_RFP]);
	}
}

void
stack_save_td(struct stack *st, struct thread *td)
{
	u_int32_t *frame;

	if (TD_IS_SWAPPED(td))
		panic("stack_save_td: swapped");
	if (TD_IS_RUNNING(td))
		panic("stack_save_td: running");

	frame = (u_int32_t *)td->td_pcb->un_32.pcb32_r11;
	stack_capture(st, frame);
}

void
stack_save(struct stack *st)
{
	u_int32_t *frame;

	frame = (u_int32_t *)__builtin_frame_address(0);
	stack_capture(st, frame);
}