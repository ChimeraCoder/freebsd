
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
#include <ieeefp.h>

fp_except_t
fpsetmask(fp_except_t mask)
{
	u_int64_t fpsr;
	u_int64_t oldmask;

	__asm __volatile("mov %0=ar.fpsr" : "=r" (fpsr));
	oldmask = ~fpsr & 0x3d;
	fpsr = (fpsr & ~0x3d) | (~mask & 0x3d);
	__asm __volatile("mov ar.fpsr=%0" :: "r" (fpsr));
	return (oldmask);
}