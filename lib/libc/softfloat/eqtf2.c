
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
/* $NetBSD: eqtf2.c,v 1.1 2011/01/17 10:08:35 matt Exp $ */
/*
 * Written by Matt Thomas, 2011.  This file is in the Public Domain.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

#ifdef FLOAT128
flag __eqtf2(float128, float128);

flag
__eqtf2(float128 a, float128 b)
{

	/* libgcc1.c says !(a == b) */
	return !float128_eq(a, b);
}
#endif /* FLOAT128 */