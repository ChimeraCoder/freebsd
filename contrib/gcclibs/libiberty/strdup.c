
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
@deftypefn Supplemental char* strdup (const char *@var{s})

Returns a pointer to a copy of @var{s} in memory obtained from
@code{malloc}, or @code{NULL} if insufficient memory was available.

@end deftypefn

*/

#include <ansidecl.h>
#include <stddef.h>

extern size_t	strlen (const char*);
extern PTR	malloc (size_t);
extern PTR	memcpy (PTR, const PTR, size_t);

char *
strdup(const char *s)
{
  size_t len = strlen (s) + 1;
  char *result = (char*) malloc (len);
  if (result == (char*) 0)
    return (char*) 0;
  return (char*) memcpy (result, s, len);
}