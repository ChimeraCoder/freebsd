
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
/* Copyright (C) 1990, 2000, 2002 Free Software Foundation
    written by Doug Lea <dl@rocky.oswego.edu>
*/

#include <hash.h>

/*
 Some useful hash function.
 It's not a particularly good hash function (<< 5 would be better than << 4),
 but people believe in it because it comes from Dragon book.
*/

unsigned int
hashpjw (const unsigned char *x, unsigned int len) // From Dragon book, p436
{
  unsigned int h = 0;
  unsigned int g;

  for (; len > 0; len--)
    {
      h = (h << 4) + *x++;
      if ((g = h & 0xf0000000) != 0)
        h = (h ^ (g >> 24)) ^ g;
    }
  return h;
}