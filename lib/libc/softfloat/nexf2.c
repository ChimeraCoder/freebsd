
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
/* $NetBSD: nexf2.c,v 1.2 2004/09/27 10:16:24 he Exp $ */
/*
 * Written by Ben Harris, 2000.  This file is in the Public Domain.
 */

#include "softfloat-for-gcc.h"
#include "milieu.h"
#include "softfloat.h"

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#ifdef FLOATX80

flag __nexf2(floatx80, floatx80);

flag
__nexf2(floatx80 a, floatx80 b)
{

	/* libgcc1.c says a != b */
	return !floatx80_eq(a, b);
}
#endif /* FLOATX80 */