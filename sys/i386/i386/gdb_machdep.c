
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
#include <sys/kdb.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/signal.h>

#include <machine/gdb_machdep.h>
#include <machine/pcb.h>
#include <machine/trap.h>
#include <machine/frame.h>
#include <machine/endian.h>

#include <gdb/gdb.h>

void *
gdb_cpu_getreg(int regnum, size_t *regsz)
{

	*regsz = gdb_cpu_regsz(regnum);

	if (kdb_thread  == curthread) {
		switch (regnum) {
		case 0:	return (&kdb_frame->tf_eax);
		case 1:	return (&kdb_frame->tf_ecx);
		case 2:	return (&kdb_frame->tf_edx);
		}
	}
	switch (regnum) {
	case 3:  return (&kdb_thrctx->pcb_ebx);
	case 4:  return (&kdb_thrctx->pcb_esp);
	case 5:  return (&kdb_thrctx->pcb_ebp);
	case 6:  return (&kdb_thrctx->pcb_esi);
	case 7:  return (&kdb_thrctx->pcb_edi);
	case 8:  return (&kdb_thrctx->pcb_eip);
	}
	return (NULL);
}

void
gdb_cpu_setreg(int regnum, void *val)
{

	switch (regnum) {
	case GDB_REG_PC:
		kdb_thrctx->pcb_eip = *(register_t *)val;
		if (kdb_thread  == curthread)
			kdb_frame->tf_eip = *(register_t *)val;
	}
}

int
gdb_cpu_signal(int type, int code)
{

	switch (type & ~T_USER) {
	case 0: return (SIGFPE);	/* Divide by zero. */
	case 1: return (SIGTRAP);	/* Debug exception. */
	case 3: return (SIGTRAP);	/* Breakpoint. */
	case 4: return (SIGURG);	/* into instr. (overflow). */
	case 5: return (SIGURG);	/* bound instruction. */
	case 6: return (SIGILL);	/* Invalid opcode. */
	case 7: return (SIGFPE);	/* Coprocessor not present. */
	case 8: return (SIGEMT);	/* Double fault. */
	case 9: return (SIGSEGV);	/* Coprocessor segment overrun. */
	case 10: return (SIGTRAP);	/* Invalid TSS (also single-step). */
	case 11: return (SIGSEGV);	/* Segment not present. */
	case 12: return (SIGSEGV);	/* Stack exception. */
	case 13: return (SIGSEGV);	/* General protection. */
	case 14: return (SIGSEGV);	/* Page fault. */
	case 16: return (SIGEMT);	/* Coprocessor error. */
	}
	return (SIGEMT);
}