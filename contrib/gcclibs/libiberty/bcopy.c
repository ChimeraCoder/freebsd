
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
/* bcopy -- copy memory regions of arbitary length
@deftypefn Supplemental void bcopy (char *@var{in}, char *@var{out}, int @var{length})

Copies @var{length} bytes from memory region @var{in} to region
@var{out}.  The use of @code{bcopy} is deprecated in new programs.

@end deftypefn

*/

#include <stddef.h>

void
bcopy (const void *src, void *dest, size_t len)
{
  if (dest < src)
    {
      const char *firsts = src;
      char *firstd = dest;
      while (len--)
	*firstd++ = *firsts++;
    }
  else
    {
      const char *lasts = (const char *)src + (len-1);
      char *lastd = (char *)dest + (len-1);
      while (len--)
        *lastd-- = *lasts--;
    }
}