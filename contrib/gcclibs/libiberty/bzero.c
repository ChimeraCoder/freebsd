
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
/* Portable version of bzero for systems without it.   This function is in the public domain.  */

/*

@deftypefn Supplemental void bzero (char *@var{mem}, int @var{count})

Zeros @var{count} bytes starting at @var{mem}.  Use of this function
is deprecated in favor of @code{memset}.

@end deftypefn

*/

#include <stddef.h>

extern void *memset(void *, int, size_t);

void
bzero (void *to, size_t count)
{
  memset (to, 0, count);
}