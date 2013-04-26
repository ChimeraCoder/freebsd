
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
/* calloc -- allocate memory which has been initialized to zero.   This function is in the public domain. */

/*

@deftypefn Supplemental void* calloc (size_t @var{nelem}, size_t @var{elsize})

Uses @code{malloc} to allocate storage for @var{nelem} objects of
@var{elsize} bytes each, then zeros the memory.

@end deftypefn

*/
 
#include "ansidecl.h"
#include <stddef.h>

/* For systems with larger pointers than ints, this must be declared.  */
PTR malloc (size_t);
void bzero (PTR, size_t);

PTR
calloc (size_t nelem, size_t elsize)
{
  register PTR ptr;  

  if (nelem == 0 || elsize == 0)
    nelem = elsize = 1;
  
  ptr = malloc (nelem * elsize);
  if (ptr) bzero (ptr, nelem * elsize);
  
  return ptr;
}