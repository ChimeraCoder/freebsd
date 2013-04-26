
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

#include <sys/types.h>
#include <ieeefp.h>

fp_rnd_t
fpsetround(fp_rnd_t rnd)
{
	uint64_t fpsr;
	fp_rnd_t prev;

	__asm __volatile("mov %0=ar.fpsr" : "=r"(fpsr));
	prev = (fp_rnd_t)((fpsr >> 10) & 3);
	fpsr = (fpsr & ~0xC00ULL) | ((unsigned int)rnd << 10);
	__asm __volatile("mov ar.fpsr=%0" :: "r"(fpsr));
	return (prev);
}