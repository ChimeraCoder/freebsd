
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

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "roken.h"

#if !HAVE_DECL_ENVIRON
extern char **environ;
#endif

/*
 * unsetenv --
 */
ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
unsetenv(const char *name)
{
  int len;
  const char *np;
  char **p;

  if (name == 0 || environ == 0)
    return;

  for (np = name; *np && *np != '='; np++)
    /* nop */;
  len = np - name;

  for (p = environ; *p != 0; p++)
    if (strncmp(*p, name, len) == 0 && (*p)[len] == '=')
      break;

  for (; *p != 0; p++)
    *p = *(p + 1);
}