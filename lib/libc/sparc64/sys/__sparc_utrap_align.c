
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
#include <machine/instr.h>

#include <signal.h>

#include "__sparc_utrap_private.h"

static u_long
__unaligned_load(u_char *p, int size)
{
	u_long val;
	int i;

	val = 0;
	for (i = 0; i < size; i++)
		val = (val << 8) | p[i];
	return (val);
}

static void
__unaligned_store(u_char *p, u_long val, int size)
{
	int i;

	for (i = 0; i < size; i++)
		p[i] = val >> ((size - i - 1) * 8);
}

int
__unaligned_fixup(struct utrapframe *uf)
{
	u_char *addr;
	u_long val;
	u_int insn;
	int sig;

	sig = 0;
	addr = (u_char *)uf->uf_sfar;
	insn = *(u_int *)uf->uf_pc;
	flushw();
	switch (IF_OP(insn)) {
	case IOP_LDST:
		switch (IF_F3_OP3(insn)) {
		case INS3_LDUH:
			val = __unaligned_load(addr, 2);
			__emul_store_reg(uf, IF_F3_RD(insn), val);
			break;
		case INS3_LDUW:
			val = __unaligned_load(addr, 4);
			__emul_store_reg(uf, IF_F3_RD(insn), val);
			break;
		case INS3_LDX:
			val = __unaligned_load(addr, 8);
			__emul_store_reg(uf, IF_F3_RD(insn), val);
			break;
		case INS3_LDSH:
			val = __unaligned_load(addr, 2);
			__emul_store_reg(uf, IF_F3_RD(insn),
			    IF_SEXT(val, 16));
			break;
		case INS3_LDSW:
			val = __unaligned_load(addr, 4);
			__emul_store_reg(uf, IF_F3_RD(insn),
			    IF_SEXT(val, 32));
			break;
		case INS3_STH:
			val = __emul_fetch_reg(uf, IF_F3_RD(insn));
			__unaligned_store(addr, val, 2);
			break;
		case INS3_STW:
			val = __emul_fetch_reg(uf, IF_F3_RD(insn));
			__unaligned_store(addr, val, 4);
			break;
		case INS3_STX:
			val = __emul_fetch_reg(uf, IF_F3_RD(insn));
			__unaligned_store(addr, val, 8);
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