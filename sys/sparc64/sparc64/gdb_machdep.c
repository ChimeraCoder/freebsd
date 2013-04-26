
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
#include <sys/signal.h>

#include <machine/asm.h>
#include <machine/gdb_machdep.h>
#include <machine/pcb.h>
#include <machine/reg.h>

#include <gdb/gdb.h>

void *
gdb_cpu_getreg(int regnum, size_t *regsz)
{
	static uint64_t synth;

	*regsz = gdb_cpu_regsz(regnum);
	switch (regnum) {
		/* 0-7: g0-g7 */
		/* 8-15: o0-o7 */
	case 14:
		synth = kdb_thrctx->pcb_sp - CCFSZ;
		return (&synth);
		/* 16-23: l0-l7 */
		/* 24-31: i0-i7 */
	case 30: return (&kdb_thrctx->pcb_sp);
		/* 32-63: f0-f31 */
		/* 64-79: f32-f62 (16 double FP) */	
	case 80: return (&kdb_thrctx->pcb_pc);
	}
	return (NULL);
}

void
gdb_cpu_setreg(int regnum, void *val)
{
	switch (regnum) {
	}
}