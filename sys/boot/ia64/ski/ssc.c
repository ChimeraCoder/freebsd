
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

#include <stand.h>
#include "libski.h"

/*
 * Ugh... Work around a bug in the Linux version of ski for SSC_GET_RTC. The
 * PSR.dt register is not preserved properly and causes further memory
 * references to be done without translation. All we need to do is preserve
 * PSR.dt across the SSC call. We do this by saving and restoring psr.l
 * completely.
 */
u_int64_t
ssc(u_int64_t in0, u_int64_t in1, u_int64_t in2, u_int64_t in3, int which)
{
	register u_int64_t psr;
	register u_int64_t ret0 __asm("r8");

	__asm __volatile("mov %0=psr;;" : "=r"(psr));
	__asm __volatile("mov r15=%1\n\t"
			 "break 0x80000;;"
			 : "=r"(ret0)
			 : "r"(which), "r"(in0), "r"(in1), "r"(in2), "r"(in3));
	__asm __volatile("mov psr.l=%0;; srlz.d" :: "r"(psr));
	return ret0;
}