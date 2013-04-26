
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
@deftypefn Supplemental int vprintf (const char *@var{format}, va_list @var{ap})
@deftypefnx Supplemental int vfprintf (FILE *@var{stream}, const char *@var{format}, va_list @var{ap})
@deftypefnx Supplemental int vsprintf (char *@var{str}, const char *@var{format}, va_list @var{ap})

These functions are the same as @code{printf}, @code{fprintf}, and
@code{sprintf}, respectively, except that they are called with a
@code{va_list} instead of a variable number of arguments.  Note that
they do not call @code{va_end}; this is the application's
responsibility.  In @libib{} they are implemented in terms of the
nonstandard but common function @code{_doprnt}.

@end deftypefn

*/

#include <ansidecl.h>
#include <stdarg.h>
#include <stdio.h>
#undef vprintf
int
vprintf (const char *format, va_list ap)
{
  return vfprintf (stdout, format, ap);
}