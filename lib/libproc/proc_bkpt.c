
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
#include <sys/wait.h>
#include <machine/_inttypes.h>

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <errno.h>
#include "_libproc.h"

#if defined(__i386__) || defined(__amd64__)
#define BREAKPOINT_INSTR	0xcc	/* int 0x3 */
#define	BREAKPOINT_INSTR_SZ	1
#elif defined(__mips__)
#define BREAKPOINT_INSTR	0xd	/* break */
#define	BREAKPOINT_INSTR_SZ	4
#elif defined(__powerpc__)
#define BREAKPOINT_INSTR	0x7fe00008	/* trap */
#define BREAKPOINT_INSTR_SZ 4
#else
#error "Add support for your architecture"
#endif

int
proc_bkptset(struct proc_handle *phdl, uintptr_t address,
    unsigned long *saved)
{
	struct ptrace_io_desc piod;
	unsigned long paddr, caddr;

	*saved = 0;
	if (phdl->status == PS_DEAD || phdl->status == PS_UNDEAD ||
	    phdl->status == PS_IDLE) {
		errno = ENOENT;
		return (-1);
	}

	/*
	 * Read the original instruction.
	 */
	caddr = address;
	paddr = 0;
	piod.piod_op = PIOD_READ_I;
	piod.piod_offs = (void *)caddr;
	piod.piod_addr = &paddr;
	piod.piod_len  = BREAKPOINT_INSTR_SZ;
	if (ptrace(PT_IO, proc_getpid(phdl), (caddr_t)&piod, 0) < 0) {
		DPRINTF("ERROR: couldn't read instruction at address 0x%" PRIuPTR,
		    address);
		return (-1);
	}
	*saved = paddr;
	/*
	 * Write a breakpoint instruction to that address.
	 */
	caddr = address;
	paddr = BREAKPOINT_INSTR;
	piod.piod_op = PIOD_WRITE_I;
	piod.piod_offs = (void *)caddr;
	piod.piod_addr = &paddr;
	piod.piod_len  = BREAKPOINT_INSTR_SZ;
	if (ptrace(PT_IO, proc_getpid(phdl), (caddr_t)&piod, 0) < 0) {
		warn("ERROR: couldn't write instruction at address 0x%" PRIuPTR,
		    address);
		return (-1);
	}

	return (0);
}

int
proc_bkptdel(struct proc_handle *phdl, uintptr_t address,
    unsigned long saved)
{
	struct ptrace_io_desc piod;
	unsigned long paddr, caddr;

	if (phdl->status == PS_DEAD || phdl->status == PS_UNDEAD ||
	    phdl->status == PS_IDLE) {
		errno = ENOENT;
		return (-1);
	}
	DPRINTF("removing breakpoint at 0x%lx\n", address);
	/*
	 * Overwrite the breakpoint instruction that we setup previously.
	 */
	caddr = address;
	paddr = saved;
	piod.piod_op = PIOD_WRITE_I;
	piod.piod_offs = (void *)caddr;
	piod.piod_addr = &paddr;
	piod.piod_len  = BREAKPOINT_INSTR_SZ;
	if (ptrace(PT_IO, proc_getpid(phdl), (caddr_t)&piod, 0) < 0) {
		DPRINTF("ERROR: couldn't write instruction at address 0x%" PRIuPTR,
		    address);
		return (-1);
	}
 
	return (0);
}

/*
 * Decrement pc so that we delete the breakpoint at the correct
 * address, i.e. at the BREAKPOINT_INSTR address.
 */
void
proc_bkptregadj(unsigned long *pc)
{
	*pc = *pc - BREAKPOINT_INSTR_SZ;
}

/*
 * Step over the breakpoint.
 */
int
proc_bkptexec(struct proc_handle *phdl, unsigned long saved)
{
	unsigned long pc;
	unsigned long samesaved;
	int status;

	if (proc_regget(phdl, REG_PC, &pc) < 0) {
		warn("ERROR: couldn't get PC register");
		return (-1);
	}
	proc_bkptregadj(&pc);
	if (proc_bkptdel(phdl, pc, saved) < 0) {
		warn("ERROR: couldn't delete breakpoint");
		return (-1);
	}
	/*
	 * Go back in time and step over the new instruction just
	 * set up by proc_bkptdel().
	 */
	proc_regset(phdl, REG_PC, pc);
	if (ptrace(PT_STEP, proc_getpid(phdl), (caddr_t)1, 0) < 0) {
		warn("ERROR: ptrace step failed");
		return (-1);
	}
	proc_wstatus(phdl);
	status = proc_getwstat(phdl);
	if (!WIFSTOPPED(status)) {
		warn("ERROR: don't know why process stopped");
		return (-1);
	}
	/*
	 * Restore the breakpoint. The saved instruction should be
	 * the same as the one that we were passed in.
	 */
	if (proc_bkptset(phdl, pc, &samesaved) < 0) {
		warn("ERROR: couldn't restore breakpoint");
		return (-1);
	}
	assert(samesaved == saved);

	return (0);
}