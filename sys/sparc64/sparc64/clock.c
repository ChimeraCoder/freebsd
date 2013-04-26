
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
#include <sys/pcpu.h>
#include <sys/proc.h>
#include <sys/sched.h>

#include <machine/cpu.h>
#include <machine/cpufunc.h>

void
DELAY(int usec)
{
	u_long end;

	if (usec < 0)
		return;

	/*
	 * We avoid being migrated to another CPU with a possibly
	 * unsynchronized TICK timer while spinning.
	 */
	sched_pin();

	end = rd(tick) + (u_long)usec * PCPU_GET(clock) / 1000000;
	while (rd(tick) < end)
		cpu_spinwait();

	sched_unpin();
}