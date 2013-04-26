
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
/*	$NetBSD: fpgetsticky.c,v 1.2 2002/01/13 21:45:50 thorpej Exp $	*/
/*
 * Written by J.T. Conklin, Apr 10, 1995
 * Public domain.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <machine/fsr.h>
#include <ieeefp.h>

fp_except_t
fpgetsticky()
{
	unsigned int x;

	__asm__("st %%fsr,%0" : "=m" (x));
	return (FSR_GET_AEXC(x));
}