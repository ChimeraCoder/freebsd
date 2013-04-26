
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

#ifndef lint
#if 0
static char sccsid[] = "@(#)pattern.c	8.1 (Berkeley) 5/31/93";
#endif
static const char rcsid[] =
 "$FreeBSD$";
#endif /* not lint */

/*
 * pattern - the Eratosthenes sieve on odd numbers for 3,5,7,11 and 13
 *
 * By: Landon Curt Noll                             chongo@toad.com
 *
 *   chongo <for a good prime call: 391581 * 2^216193 - 1> /\oo/\
 *
 * To avoid excessive sieves for small factors, we use the table below to
 * setup our sieve blocks.  Each element represents an odd number starting
 * with 1.  All non-zero elements are factors of 3, 5, 7, 11 and 13.
 */

#include <stddef.h>

#include "primes.h"

const char pattern[] = {
1,0,0,0,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,
1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0,0,0,1,0,1,1,0,1,
0,0,1,1,0,0,0,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,
1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,
1,0,0,1,0,1,0,0,0,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,
0,0,0,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0,0,
1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,
0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,
1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,
0,0,0,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,
1,0,1,0,0,0,0,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,
1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,0,1,0,0,1,0,1,
1,0,1,0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,0,
1,0,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,0,0,1,
1,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,
1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,0,0,0,0,1,0,1,
0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0,0,1,
1,0,1,1,0,1,0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,
0,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,
0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,1,0,0,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,
0,0,0,1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,
1,0,0,1,0,1,0,0,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,
1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,0,0,0,
0,0,1,1,0,1,0,0,0,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,0,0,0,1,0,0,1,0,1,
0,0,0,0,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,
1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,
1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,
1,0,1,0,0,0,0,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,
0,0,1,1,0,1,0,0,0,0,0,0,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,0,0,0,1,0,0,1,
0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,
0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,
1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,1,0,1,1,0,0,
1,0,1,0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,
1,0,0,0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,0,1,0,1,0,0,1,
0,0,1,1,0,0,0,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,
1,0,0,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,0,0,1,
1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,1,
0,0,1,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,
1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,
0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,0,0,0,0,1,0,1,
1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,0,0,
1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,0,0,0,1,0,0,0,
1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,
0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,0,0,1,
1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,
1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,
1,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,0,0,0,1,1,0,0,
1,0,1,1,0,0,0,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1,
0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0,0,1,
0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,
0,0,0,1,0,0,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,0,0,0,0,1,0,0,1,0,1,1,0,1,
0,0,0,1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
1,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,
1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,0,0,1,
1,0,0,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0,0,
0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,
0,0,1,0,0,0,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,
1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,
0,0,0,1,0,1,0,0,1,0,0,0,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,0,0,1,1,0,1,
1,0,1,0,0,0,0,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,
0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,0,1,0,0,1,0,1,
0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,
1,0,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,0,0,0,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,0,0,0,0,1,1,0,0,
1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,0,1,0,0,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,0,0,0,0,0,0,0,1,0,1,
1,0,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,
1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,
0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,
1,0,1,0,0,0,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,
0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,
1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,
1,0,0,1,0,0,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,0,0,0,
1,0,1,1,0,1,0,0,0,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,
0,0,0,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,1,0,0,0,
1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,
1,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,
0,0,1,1,0,0,0,0,1,0,0,0,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1,
0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,0,0,0,0,1,0,0,0,0,0,
1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,
0,0,0,1,0,1,0,0,1,1,0,1,0,0,0,0,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,0,1,0,1,
1,0,1,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,
0,0,0,1,0,0,0,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
1,0,0,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,0,0,0,0,0,1,
1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,
1,0,1,0,0,1,0,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,
0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,
0,0,1,0,0,0,0,0,1,1,0,0,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,0,0,0,0,1,0,0,
1,0,1,1,0,0,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,0,
1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,
1,0,1,0,0,0,0,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,
0,0,1,0,0,0,0,0,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,0,0,1,
0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,0,0,0,1,0,0,0,0,1,
1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,1,0,0,0,0,1,1,0,0,
1,0,0,1,0,1,0,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,
0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,0,0,1,0,0,1,
1,0,0,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,
0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,
1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,0,1,0,1,1,0,0,
0,0,1,1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,
1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,
1,0,0,1,0,0,0,0,0,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,0,0,0,0,0,1,1,0,1,
1,0,0,0,0,1,0,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,0,0,0,0,0,0,0,
1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,0,
0,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,0,0,1,1,0,1,
1,0,0,0,0,0,0,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,0,0,1,0,0,0,0,0,1,1,0,0,
1,0,1,1,0,0,0,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1,
0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,0,1,0,0,0,0,1,
1,0,1,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,
0,0,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,0,0,1,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,0,0,0,0,0,1,1,0,0,
1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,1,
0,0,0,1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,0,0,0,1,0,0,1,0,1,
1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,
1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,
0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,
0,0,1,1,0,0,0,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,
0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,
1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,
1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,
1,0,1,0,0,0,0,0,0,1,0,1,0,0,0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,0,0,1,0,0,0,
1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,0,0,0,1,0,1,0,0,1,
0,0,0,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,0,0,0,1,
1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,
0,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,
1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,
1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,1,0,1,1,0,1,
0,0,1,0,0,1,0,0,0,0,0,1,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,
1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,0,
0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,
1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,0,0,0,1,0,0,0,0,1,
1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,1,
1,0,1,0,0,1,0,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,0,0,0,
1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,0,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,
1,0,1,0,0,0,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,0,0,1,1,0,0,1,0,1,0,0,0,
1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,
1,0,0,0,0,1,0,0,0,0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,0,0,0,1,1,0,0,
1,0,1,1,0,0,0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1,
0,0,1,0,0,0,0,0,0,1,0,1,1,0,0,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0,0,1,
1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,
0,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0,0,0,0,1,1,0,0,
1,0,1,1,0,1,0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,
0,0,0,1,0,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,
1,0,0,1,0,0,0,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,
1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,
0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,0,1,0,1,
0,0,1,0,0,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,
1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,
1,0,0,1,0,1,0,0,0,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,
1,0,0,0,0,0,0,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0,0,1,0,0,
1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,
0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,
0,0,0,0,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,0,0,0,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,
1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,
1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,
1,0,0,0,0,1,0,0,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,0,0,1,
0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,0,0,0,0,1,0,0,
1,0,0,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,0,
0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,0,0,0,0,1,0,0,1,0,1,
1,0,0,1,0,1,0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,
1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,
0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,0,0,0,
1,0,1,1,0,0,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,
0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,0,
1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,
1,0,0,0,0,1,0,0,0,0,0,1,1,0,0,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,0,0,0,1,0,0,0,
1,0,1,1,0,0,0,0,0,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,0,0,0,0,0,1,1,0,1,
0,0,0,1,0,0,0,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,0,0,1,1,0,0,0,0,0,1,0,0,0,0,1,
1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,
0,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,
0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,0,0,1,1,0,1,
0,0,0,1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,
1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,
1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,
1,0,1,0,0,1,0,0,0,0,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,
0,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,0,0,1,
0,0,1,0,0,0,0,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,
1,0,0,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,0,0,0,0,0,1,1,0,0,1,0,0,0,0,1,
1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,1,0,1,
1,0,1,0,0,0,0,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,
0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,0,0,0,0,
1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,
1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,
0,0,1,0,0,0,0,0,1,1,0,1,0,0,0,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,0,0,0,0,0,0,1,
1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,0,0,0,1,0,1,0,0,1,
1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,
0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,0,0,0,0,1,0,0,
1,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,
0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,
0,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,0,0,1,0,0,1,
1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,0,0,0,1,0,0,0,
1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,1,0,1,1,0,1,
0,0,1,1,0,0,0,0,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,0,
1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,
1,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,0,0,1,1,0,1,0,0,0,0,0,0,1,0,0,
1,0,1,1,0,0,0,0,1,0,0,1,0,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1,
0,0,1,1,0,0,0,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,0,0,0,1,0,0,0,0,1,
1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,
0,0,0,1,0,1,0,0,1,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,0,0,0,1,0,1,
1,0,1,0,0,0,0,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,0,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,
0,0,0,1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,0,1,0,0,1,0,1,
1,0,1,1,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,
1,0,0,0,0,1,0,0,0,1,0,1,0,0,0,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,
1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,
0,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,
0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,0,0,1,1,0,0,0,0,0,1,0,0,1,0,0,
1,0,1,1,0,1,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,
1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,
0,0,1,0,0,0,0,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,0,0,0,
0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,
1,0,0,0,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,0,0,0,
1,0,0,1,0,1,0,0,0,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,
0,0,0,1,0,0,0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,0,0,0,1,0,1,0,0,0,0,0,0,1,0,1,
1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,
1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,
0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,
0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,0,0,0,1,1,0,0,
0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,
1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,
1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,
1,0,1,0,0,1,0,0,0,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,0,0,0,
1,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,
0,0,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
1,0,0,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,0,0,0,0,
1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,1,
1,0,0,0,0,1,0,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,
1,0,1,1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1,
0,0,1,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,1,
1,0,1,1,0,0,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,0,
0,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,
0,0,0,0,0,0,0,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,0,1,0,0,0,0,1,
1,0,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,0,0,0,0,0,1,0,0,1,
1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,1,
1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,1,0,0,
0,0,1,1,0,1,0,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,
0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,
0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,0,0,1,
1,0,0,1,0,0,0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,
1,0,1,0,0,0,0,0,0,1,0,1,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,0,1,0,1,0,0,1,
0,0,1,1,0,0,0,0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,
1,0,0,0,0,1,0,0,0,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,
1,0,0,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,1,1,0,0,0,0,0,1,0,0,
1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,
1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,
0,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,0,0,1,1,0,1,
0,0,1,0,0,0,0,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,0,0,1,1,0,0,0,0,0,1,0,0,
1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,
0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,0,1,0,0,1,0,1,
1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,
1,0,0,0,0,0,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,0,0,1,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,0,0,0,1,0,0,0,
1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0,0,1,0,1,
1,0,1,0,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,1,0,0,1,0,1,0,0,0,
1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,
0,0,0,0,0,1,0,0,0,0,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,
1,0,1,1,0,0,0,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,0,0,0,1,0,0,0,0,1,1,0,0,
0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0,0,1,
1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,
0,0,0,1,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,0,0,0,
1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,0,0,0,1,0,1,1,0,1,
0,0,0,1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,0,0,1,0,0,1,
1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,
1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,
0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,0,0,0,1,0,1,
0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,0,
1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,
1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,0,1,
1,0,1,0,0,0,0,0,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,0,0,1,0,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,
0,0,1,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
0,0,0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,0,0,0,1,
1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,
1,0,1,0,0,1,0,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,
1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,
1,0,1,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,0,0,0,1,0,0,1,0,1,0,0,0,
1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,
0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,0,0,0,1,0,0,
1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,
0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,0,0,1,
1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,
1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,1,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,0,0,0,
1,0,1,1,0,1,0,0,1,0,0,0,0,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,0,0,1,0,0,0,
1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,
1,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,
1,0,1,1,0,0,0,0,1,0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,0,0,0,1,1,0,1,
0,0,1,1,0,0,0,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0,0,1,
1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,
0,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,
1,0,0,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0,0,
1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,
0,0,0,1,0,0,0,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,
0,0,0,1,0,1,0,0,0,1,0,0,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,1,
1,0,1,0,0,0,0,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,
0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,
0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,0,1,0,1,0,0,0,1,0,0,1,0,0,
1,0,1,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,
1,0,0,0,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,0,0,1,
1,0,1,0,0,0,0,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,0,0,0,1,1,0,0,
1,0,0,1,0,1,0,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,0,0,1,
0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,1,0,1,
0,0,1,1,0,1,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,
1,0,0,0,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,
0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,
1,0,0,1,0,0,0,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,0,0,0,0,0,0,1,0,1,1,0,0,
0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,
1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,
1,0,0,1,0,1,0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,
0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,0,0,0,0,0,0,0,
1,0,1,0,0,1,0,0,0,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,0,0,0,1,0,1,1,0,0,
0,0,0,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,
1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,0,0,0,1,0,1,0,0,1,
1,0,0,1,0,0,0,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,
1,0,1,0,0,1,0,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,1,0,1,1,0,0,1,0,0,0,0,1,0,0,0,
0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,0,0,1,1,0,1,
0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,0,
1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,1,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,1,0,0,0,
1,0,0,1,0,1,0,0,1,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,1,0,1,1,0,1,
1,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,1,0,0,0,0,0,1,1,0,0,
1,0,1,0,0,0,0,0,1,0,0,1,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,
0,0,1,1,0,0,0,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,1,0,0,0,0,0,1,0,0,0,0,1,
1,0,0,1,0,1,0,0,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,0,0,0,1,0,1,1,0,0,1,0,0,0,0,1,
0,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,0,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,0,1,0,1,
1,0,1,0,0,1,0,0,0,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,
1,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,1,0,1,
0,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,0,0,0,0,1,0,1,
1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0,1,1,0,1,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,0,0,0,
1,0,0,1,0,1,0,0,0,1,0,1,0,0,1,0,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,1,0,1,0,0,1,
1,0,1,0,0,1,0,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,0,0,0,1,1,0,0,
0,0,1,1,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,1,
0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,1,0,0,1,0,1,0,0,1,1,0,1,0,0,0,1,0,0,0,0,0,
1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,1,0,1,1,0,0,0,0,1,1,0,0,0,0,1,0,0,0,1,0,1,0,0,1,
1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,1,0,0,1,0,0,0,1,0,0,1,0,1,
1,0,1,0,0,0,0,0,0,1,0,1,0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,0,1,0,0,0,0,1,1,0,0,
1,0,1,1,0,1,0,0,0,0,0,1,0,0,0,0,0,1,1,0,1,1,0,0,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,
0,0,1,1,0,0,0,0,1,1,0,0,1,0,1,0,0,0,0,0,0,1,0,1,1,0,1,1,0,1,0,0,0,1,0,0,1,0,1,
0,0,1,1,0,1,0,0,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0,0,0,0,0,0,0,1
};
const size_t pattern_size = (sizeof(pattern)/sizeof(pattern[0]));