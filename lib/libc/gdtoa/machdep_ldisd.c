
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
 * Machine-dependent glue to integrate David Gay's gdtoa
 * package into libc for architectures where a long double
 * is the same as a double, such as the Alpha.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "gdtoaimp.h"
#undef strtold_l

long double
strtold_l(const char * __restrict s, char ** __restrict sp, locale_t locale)
{

	return strtod_l(s, sp, locale);
}