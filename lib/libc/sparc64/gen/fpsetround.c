
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
/*	$NetBSD: fpsetround.c,v 1.2 2002/01/13 21:45:51 thorpej Exp $	*/
/*
 * Written by J.T. Conklin, Apr 10, 1995
 * Public domain.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <machine/fsr.h>
#include <ieeefp.h>

fp_rnd_t
fpsetround(rnd_dir)
	fp_rnd_t rnd_dir;
{
	unsigned int old;
	unsigned int new;

	__asm__("st %%fsr,%0" : "=m" (old));

	new = old;
	new &= ~FSR_RD_MASK;
	new |= FSR_RD((unsigned int)rnd_dir & 0x03);

	__asm__("ld %0,%%fsr" : : "m" (new));

	return ((fp_rnd_t)FSR_GET_RD(old));
}