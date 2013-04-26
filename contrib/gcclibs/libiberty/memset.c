
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
/* memset   This implementation is in the public domain.  */

/*

@deftypefn Supplemental void* memset (void *@var{s}, int @var{c}, size_t @var{count})

Sets the first @var{count} bytes of @var{s} to the constant byte
@var{c}, returning a pointer to @var{s}.

@end deftypefn

*/

#include <ansidecl.h>
#include <stddef.h>

PTR
memset (PTR dest, register int val, register size_t len)
{
  register unsigned char *ptr = (unsigned char*)dest;
  while (len-- > 0)
    *ptr++ = val;
  return dest;
}