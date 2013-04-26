
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
@deftypefn Supplemental int waitpid (int @var{pid}, int *@var{status}, int)

This is a wrapper around the @code{wait} function.  Any ``special''
values of @var{pid} depend on your implementation of @code{wait}, as
does the return value.  The third argument is unused in @libib{}.

@end deftypefn

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "ansidecl.h"

/* On some systems (such as WindISS), you must include <sys/types.h>
   to get the definition of "pid_t" before you include <sys/wait.h>.  */
#include <sys/types.h>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

pid_t
waitpid (pid_t pid, int *stat_loc, int options ATTRIBUTE_UNUSED)
{
  for (;;)
    {
      int wpid = wait(stat_loc);
      if (wpid == pid || wpid == -1)
	return wpid;
    }
}