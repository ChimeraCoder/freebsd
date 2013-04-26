
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
#include <machine/reg.h>

#include <machine/hid.h>
#include <machine/spr.h>

#include <machine/trap.h>

#include <gdb/gdb.h>
#include <gdb/gdb_int.h>

void *
gdb_cpu_getreg(int regnum, size_t *regsz)
{

	*regsz = gdb_cpu_regsz(regnum);

	if (kdb_thread == curthread) {
		if (regnum == 0 || (regnum >= 2 && regnum <= 31))
			return (kdb_frame->fixreg + regnum);
		if (regnum == 64)
			return (&kdb_frame->srr0);
		if (regnum == 67)
			return (&kdb_frame->lr);
	}

	if (regnum == 1)
		return (&kdb_thrctx->pcb_sp);
	if (regnum >= 14 && regnum <= 31)
		return (kdb_thrctx->pcb_context + (regnum - 14));
	if (regnum == 64)
		return (&kdb_thrctx->pcb_lr);

	return (NULL);
}

void
gdb_cpu_setreg(int regnum, void *val)
{

	switch (regnum) {
	case GDB_REG_PC:
		break;
	}
}

int
gdb_cpu_signal(int vector, int dummy __unused)
{
#if defined(BOOKE)
	if (vector == EXC_DEBUG || vector == EXC_PGM)
		return (SIGTRAP);
#else
	if (vector == EXC_TRC || vector == EXC_RUNMODETRC)
		return (SIGTRAP);
#endif

	if (vector <= 255)
		return (vector);
	else
		return (SIGEMT);
}