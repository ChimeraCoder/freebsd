
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
#include <machine/db_machdep.h>
#include <machine/vmparam.h>
#include <machine/pcb.h>
#include <machine/trap.h>
#include <machine/frame.h>
#include <machine/endian.h>

#include <gdb/gdb.h>

static register_t stacktest;

void *
gdb_cpu_getreg(int regnum, size_t *regsz)
{

	*regsz = gdb_cpu_regsz(regnum);

	if (kdb_thread == curthread) {
		if (regnum < 13)
			return (&kdb_frame->tf_r0 + regnum);
		if (regnum == 13)
			return (&kdb_frame->tf_svc_sp);
		if (regnum == 14)
			return (&kdb_frame->tf_svc_lr);
		if (regnum == 15)
			return (&kdb_frame->tf_pc);
		if (regnum == 25)
			return (&kdb_frame->tf_spsr);
	}

	switch (regnum) {
	case 8:  return (&kdb_thrctx->un_32.pcb32_r8);
	case 9:  return (&kdb_thrctx->un_32.pcb32_r9);
	case 10:  return (&kdb_thrctx->un_32.pcb32_r10);
	case 11:  return (&kdb_thrctx->un_32.pcb32_r11);
	case 12:  return (&kdb_thrctx->un_32.pcb32_r12);
	case 13:  stacktest = kdb_thrctx->un_32.pcb32_sp + 5 * 4;
		  return (&stacktest);
	case 15:
		  /*
		   * On context switch, the PC is not put in the PCB, but
		   * we can retrieve it from the stack.
		   */
		  if (kdb_thrctx->un_32.pcb32_sp > KERNBASE) {
			  kdb_thrctx->un_32.pcb32_pc = *(register_t *)
			      (kdb_thrctx->un_32.pcb32_sp + 4 * 4);
			  return (&kdb_thrctx->un_32.pcb32_pc);
		  }
	}

	return (NULL);
}

void
gdb_cpu_setreg(int regnum, void *val)
{

	switch (regnum) {
	case GDB_REG_PC:
		if (kdb_thread  == curthread)
			kdb_frame->tf_pc = *(register_t *)val;
	}
}

int
gdb_cpu_signal(int type, int code)
{

	switch (type) {
	case T_BREAKPOINT: return (SIGTRAP);
	}
	return (SIGEMT);
}