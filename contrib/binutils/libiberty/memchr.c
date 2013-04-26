
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
@deftypefn Supplemental void* memchr (const void *@var{s}, int @var{c}, size_t @var{n})

This function searches memory starting at @code{*@var{s}} for the
character @var{c}.  The search only ends with the first occurrence of
@var{c}, or after @var{length} characters; in particular, a null
character does not terminate the search.  If the character @var{c} is
found within @var{length} characters of @code{*@var{s}}, a pointer
to the character is returned.  If @var{c} is not found, then @code{NULL} is
returned.

@end deftypefn

*/

#include <ansidecl.h>
#include <stddef.h>

PTR
memchr (register const PTR src_void, int c, size_t length)
{
  const unsigned char *src = (const unsigned char *)src_void;
  
  while (length-- > 0)
  {
    if (*src == c)
     return (PTR)src;
    src++;
  }
  return NULL;
}