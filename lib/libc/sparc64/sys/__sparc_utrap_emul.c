
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
#include <machine/cpufunc.h>
#include <machine/frame.h>
#include <machine/instr.h>

#include <signal.h>

#include "__sparc_utrap_private.h"
#include "fpu_reg.h"

int
__emul_insn(struct utrapframe *uf)
{
	u_long reg, res;
	u_long *addr;
	u_int insn;
	int sig;
	int rd;
	int i;

	sig = 0;
	insn = *(u_int *)uf->uf_pc;
	flushw();
	switch (IF_OP(insn)) {
	case IOP_MISC:
		switch (IF_F3_OP3(insn)) {
		case INS2_POPC:
			if (IF_F3_RS1(insn) != 0) {
				sig = SIGILL;
				break;
			}
			reg = __emul_f3_op2(uf, insn);
			for (i = 0; i < 64; i++)
				res += (reg >> i) & 1;
			__emul_store_reg(uf, IF_F3_RD(insn), res);
			break;
		default:
			sig = SIGILL;
			break;
		}
		break;
	case IOP_LDST:
		switch (IF_F3_OP3(insn)) {
		case INS3_LDQF:
			rd = INSFPdq_RN(IF_F3_RD(insn));
			addr = (u_long *)__emul_f3_memop_addr(uf, insn);
			__fpu_setreg64(rd, addr[0]);
			__fpu_setreg64(rd + 2, addr[1]);
			break;
		case INS3_STQF:
			rd = INSFPdq_RN(IF_F3_RD(insn));
			addr = (u_long *)__emul_f3_memop_addr(uf, insn);
			addr[0] = __fpu_getreg64(rd);
			addr[1] = __fpu_getreg64(rd + 2);
			break;
		default:
			sig = SIGILL;
			break;
		}
		break;
	default:
		sig = SIGILL;
		break;
	}
	return (sig);
}

u_long
__emul_fetch_reg(struct utrapframe *uf, int reg)
{
	struct frame *frm;

	if (reg == IREG_G0)
		return (0);
	else if (reg < IREG_O0)	/* global */
		return (uf->uf_global[reg]);
	else if (reg < IREG_L0)	/* out */
		return (uf->uf_out[reg - IREG_O0]);
	else {			/* local, in */
		/*
		 * The in registers are immediately after the locals in
		 * the frame.
		 */
		frm = (struct frame *)(uf->uf_out[6] + SPOFF);
		return (frm->fr_local[reg - IREG_L0]);
	}
}

void
__emul_store_reg(struct utrapframe *uf, int reg, u_long val)
{
	struct frame *frm;

	if (reg == IREG_G0)
		return;
	if (reg < IREG_O0)	/* global */
		uf->uf_global[reg] = val;
	else if (reg < IREG_L0)	/* out */
		uf->uf_out[reg - IREG_O0] = val;
	else {
		/*
		 * The in registers are immediately after the locals in
		 * the frame.
		 */
		frm = (struct frame *)(uf->uf_out[6] + SPOFF);
		frm->fr_local[reg - IREG_L0] = val;
	}
}

u_long
__emul_f3_op2(struct utrapframe *uf, u_int insn)
{

	if (IF_F3_I(insn) != 0)
		return (IF_SIMM(insn, 13));
	else
		return (__emul_fetch_reg(uf, IF_F3_RS2(insn)));
}

u_long
__emul_f3_memop_addr(struct utrapframe *uf, u_int insn)
{
	u_long addr;

	addr = __emul_f3_op2(uf, insn) + __emul_fetch_reg(uf, IF_F3_RS1(insn));
	return (addr);
}