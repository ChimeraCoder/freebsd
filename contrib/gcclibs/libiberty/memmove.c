
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
/* Wrapper to implement ANSI C's memmove using BSD's bcopy. */
/* This function is in the public domain.  --Per Bothner. */
/*

@deftypefn Supplemental void* memmove (void *@var{from}, const void *@var{to}, size_t @var{count})

Copies @var{count} bytes from memory area @var{from} to memory area
@var{to}, returning a pointer to @var{to}.

@end deftypefn

*/

#include <ansidecl.h>
#include <stddef.h>

void bcopy (const void*, void*, size_t);

PTR
memmove (PTR s1, const PTR s2, size_t n)
{
  bcopy (s2, s1, n);
  return s1;
}