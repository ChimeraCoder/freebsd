
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
#include <sys/ptrace.h>

#include <err.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "_libproc.h"

int
proc_regget(struct proc_handle *phdl, proc_reg_t reg, unsigned long *regvalue)
{
	struct reg regs;

	if (phdl->status == PS_DEAD || phdl->status == PS_UNDEAD ||
	    phdl->status == PS_IDLE) {
		errno = ENOENT;
		return (-1);
	}
	memset(&regs, 0, sizeof(regs));
	if (ptrace(PT_GETREGS, proc_getpid(phdl), (caddr_t)&regs, 0) < 0)
		return (-1);
	switch (reg) {
	case REG_PC:
#if defined(__amd64__)
		*regvalue = regs.r_rip;
#elif defined(__i386__)
		*regvalue = regs.r_eip;
#elif defined(__mips__)
		*regvalue = regs.r_regs[PC];
#elif defined(__powerpc__)
		*regvalue = regs.pc;
#endif
		break;
	case REG_SP:
#if defined(__amd64__)
		*regvalue = regs.r_rsp;
#elif defined(__i386__)
		*regvalue = regs.r_esp;
#elif defined(__mips__)
		*regvalue = regs.r_regs[SP];
#elif defined(__powerpc__)
		*regvalue = regs.fixreg[1];
#endif
		break;
	default:
		warn("ERROR: no support for reg number %d", reg);
		return (-1);
	}

	return (0);
}

int
proc_regset(struct proc_handle *phdl, proc_reg_t reg, unsigned long regvalue)
{
	struct reg regs;

	if (phdl->status == PS_DEAD || phdl->status == PS_UNDEAD ||
	    phdl->status == PS_IDLE) {
		errno = ENOENT;
		return (-1);
	}
	if (ptrace(PT_GETREGS, proc_getpid(phdl), (caddr_t)&regs, 0) < 0)
		return (-1);
	switch (reg) {
	case REG_PC:
#if defined(__amd64__)
		regs.r_rip = regvalue;
#elif defined(__i386__)
		regs.r_eip = regvalue;
#elif defined(__mips__)
		regs.r_regs[PC] = regvalue;
#elif defined(__powerpc__)
		regs.pc = regvalue;
#endif
		break;
	case REG_SP:
#if defined(__amd64__)
		regs.r_rsp = regvalue;
#elif defined(__i386__)
		regs.r_esp = regvalue;
#elif defined(__mips__)
		regs.r_regs[PC] = regvalue;
#elif defined(__powerpc__)
		regs.fixreg[1] = regvalue;
#endif
		break;
	default:
		warn("ERROR: no support for reg number %d", reg);
		return (-1);
	}
	if (ptrace(PT_SETREGS, proc_getpid(phdl), (caddr_t)&regs, 0) < 0)
		return (-1);

	return (0);
}