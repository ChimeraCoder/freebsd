
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
/*
 * Written by J.T. Conklin, Apr 10, 1995
 * Public domain.
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <machine/fsr.h>
#include <ieeefp.h>

fp_except_t
fpsetmask(mask)
	fp_except_t mask;
{
	fp_except_t old;
	fp_except_t new;

	__asm__("st %%fsr,%0" : "=m" (old));

	new = old;
	new &= ~FSR_TEM_MASK;
	new |= FSR_TEM(mask & FSR_EXC_MASK);

	__asm__("ld %0,%%fsr" : : "m" (new));

	return (FSR_GET_TEM(old));
}