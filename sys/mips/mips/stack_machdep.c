
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

#include <sys/types.h>
#include <sys/systm.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/stack.h>

#include <machine/mips_opcode.h>

#include <machine/param.h>
#include <machine/pcb.h>
#include <machine/regnum.h>

static u_register_t
stack_register_fetch(u_register_t sp, u_register_t stack_pos)
{
	u_register_t * stack = 
	    ((u_register_t *)(intptr_t)sp + (size_t)stack_pos/sizeof(u_register_t));

	return *stack;
}

static void
stack_capture(struct stack *st, u_register_t pc, u_register_t sp)
{
	u_register_t  ra = 0, i, stacksize;
	short ra_stack_pos = 0;
	InstFmt insn;

	stack_zero(st);

	for (;;) {
		stacksize = 0;
		if (pc <= (u_register_t)(intptr_t)btext)
			break;
		for (i = pc; i >= (u_register_t)(intptr_t)btext; i -= sizeof (insn)) {
			bcopy((void *)(intptr_t)i, &insn, sizeof insn);
			switch (insn.IType.op) {
			case OP_ADDI:
			case OP_ADDIU:
			case OP_DADDI:
			case OP_DADDIU:
				if (insn.IType.rs != SP || insn.IType.rt != SP)
					break;
				stacksize = -(short)insn.IType.imm;
				break;

			case OP_SW:
			case OP_SD:
				if (insn.IType.rs != SP || insn.IType.rt != RA)
					break;
				ra_stack_pos = (short)insn.IType.imm;
				break;
			default:
				break;
			}

			if (stacksize)
				break;
		}

		if (stack_put(st, pc) == -1)
			break;

		for (i = pc; !ra; i += sizeof (insn)) {
			bcopy((void *)(intptr_t)i, &insn, sizeof insn);

			switch (insn.IType.op) {
			case OP_SPECIAL:
				if((insn.RType.func == OP_JR))
				{
					if (ra >= (u_register_t)(intptr_t)btext)
						break;
					if (insn.RType.rs != RA)
						break;
					ra = stack_register_fetch(sp, 
					    ra_stack_pos);
					if (!ra)
						goto done;
					ra -= 8;
				}
				break;
			default:
				break;
			}
			/* eret */
			if (insn.word == 0x42000018)
				goto done;
		}

		if (pc == ra && stacksize == 0)
			break;

		sp += stacksize;
		pc = ra;
		ra = 0;
	}
done:
	return;
}

void
stack_save_td(struct stack *st, struct thread *td)
{
	u_register_t pc, sp;

	if (TD_IS_SWAPPED(td))
		panic("stack_save_td: swapped");
	if (TD_IS_RUNNING(td))
		panic("stack_save_td: running");

	pc = td->td_pcb->pcb_regs.pc;
	sp = td->td_pcb->pcb_regs.sp;
	stack_capture(st, pc, sp);
}

void
stack_save(struct stack *st)
{
	u_register_t pc, sp;

	if (curthread == NULL)
		panic("stack_save: curthread == NULL)");

	pc = curthread->td_pcb->pcb_regs.pc;
	sp = curthread->td_pcb->pcb_regs.sp;
	stack_capture(st, pc, sp);
}