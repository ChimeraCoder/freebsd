
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

#include <machine/frame.h>
#include <machine/md_var.h>

#include <ia64/disasm/disasm.h>

int
ia64_emulate(struct trapframe *tf, struct thread *td)
{
	struct asm_bundle bundle;
	struct asm_inst *i;
	int slot;

	if (!asm_decode(tf->tf_special.iip, &bundle))
		return (SIGILL);

	slot = ((tf->tf_special.psr & IA64_PSR_RI) == IA64_PSR_RI_0) ? 0 :
	    ((tf->tf_special.psr & IA64_PSR_RI) == IA64_PSR_RI_1) ? 1 : 2;
	if (slot == 1 && bundle.b_templ[slot] == 'L')
		slot++;

	i = bundle.b_inst + slot;
	switch (i->i_op) {
	case ASM_OP_BRL:
		/*
		 * We get the fault even if the predicate is false, so we
		 * need to check the predicate first and simply advance to
		 * the next bundle in that case.
		 */
		if (!(tf->tf_special.pr & (1UL << i->i_oper[0].o_value))) {
			tf->tf_special.psr &= ~IA64_PSR_RI;
			tf->tf_special.iip += 16;
			return (0);
		}
		/*
		 * The brl.cond is the simplest form. We only have to set
		 * the IP to the address in the instruction and return.
		 */
		if (i->i_cmpltr[0].c_type == ASM_CT_COND) {
			tf->tf_special.psr &= ~IA64_PSR_RI;
			tf->tf_special.iip += i->i_oper[1].o_value;
			return (0);
		}
		/* Sanity check... */
		if (i->i_cmpltr[0].c_type != ASM_CT_CALL)
			break;
		/*
		 * The brl.call is more difficult as we need to set-up the
		 * call properly.
		 */
		break;
	default:
		break;
	}

	return (SIGILL);
}