
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
#include <sys/libkern.h>

#include <machine/md_var.h>

#include "vmm_util.h"

boolean_t
vmm_is_intel(void)
{

	if (strcmp(cpu_vendor, "GenuineIntel") == 0)
		return (TRUE);
	else
		return (FALSE);
}

boolean_t
vmm_is_amd(void)
{
	if (strcmp(cpu_vendor, "AuthenticAMD") == 0)
		return (TRUE);
	else
		return (FALSE);
}

boolean_t
vmm_supports_1G_pages(void)
{
	unsigned int regs[4];

	/*
	 * CPUID.80000001:EDX[bit 26] = 1 indicates support for 1GB pages
	 *
	 * Both Intel and AMD support this bit.
	 */
	if (cpu_exthigh >= 0x80000001) {
		do_cpuid(0x80000001, regs);
		if (regs[3] & (1 << 26))
			return (TRUE);
	}
	return (FALSE);
}

#include <sys/proc.h>
#include <machine/frame.h>
#define	DUMP_REG(x)	printf(#x "\t\t0x%016lx\n", (long)(tf->tf_ ## x))
#define	DUMP_SEG(x)	printf(#x "\t\t0x%04x\n", (unsigned)(tf->tf_ ## x))
void
dump_trapframe(struct trapframe *tf)
{
	DUMP_REG(rdi);
	DUMP_REG(rsi);
	DUMP_REG(rdx);
	DUMP_REG(rcx);
	DUMP_REG(r8);
	DUMP_REG(r9);
	DUMP_REG(rax);
	DUMP_REG(rbx);
	DUMP_REG(rbp);
	DUMP_REG(r10);
	DUMP_REG(r11);
	DUMP_REG(r12);
	DUMP_REG(r13);
	DUMP_REG(r14);
	DUMP_REG(r15);
	DUMP_REG(trapno);
	DUMP_REG(addr);
	DUMP_REG(flags);
	DUMP_REG(err);
	DUMP_REG(rip);
	DUMP_REG(rflags);
	DUMP_REG(rsp);
	DUMP_SEG(cs);
	DUMP_SEG(ss);
	DUMP_SEG(fs);
	DUMP_SEG(gs);
	DUMP_SEG(es);
	DUMP_SEG(ds);
}