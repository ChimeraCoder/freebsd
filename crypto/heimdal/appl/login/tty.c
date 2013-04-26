
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

#include "login_locl.h"

RCSID("$Id$");

/*
 * Clean the tty name.  Return a pointer to the cleaned version.
 */

char *
clean_ttyname (char *tty)
{
  char *res = tty;

  if (strncmp (res, _PATH_DEV, strlen(_PATH_DEV)) == 0)
    res += strlen(_PATH_DEV);
  if (strncmp (res, "pty/", 4) == 0)
    res += 4;
  if (strncmp (res, "ptym/", 5) == 0)
    res += 5;
  return res;
}

/*
 * Generate a name usable as an `ut_id', typically without `tty'.
 */

char *
make_id (char *tty)
{
  char *res = tty;

  if (strncmp (res, "pts/", 4) == 0)
    res += 4;
  if (strncmp (res, "tty", 3) == 0)
    res += 3;
  return res;
}