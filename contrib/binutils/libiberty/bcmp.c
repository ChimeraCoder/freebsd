
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
/* bcmp   This function is in the public domain.  */

/*

@deftypefn Supplemental int bcmp (char *@var{x}, char *@var{y}, int @var{count})

Compares the first @var{count} bytes of two areas of memory.  Returns
zero if they are the same, nonzero otherwise.  Returns zero if
@var{count} is zero.  A nonzero result only indicates a difference,
it does not indicate any sorting order (say, by having a positive
result mean @var{x} sorts before @var{y}).

@end deftypefn

*/

#include <stddef.h>

extern int memcmp(const void *, const void *, size_t);

int
bcmp (const void *s1, const void *s2, size_t count)
{
  return memcmp (s1, s2, count);
}