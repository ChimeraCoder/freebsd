
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
@deftypefn Supplemental char* tmpnam (char *@var{s})

This function attempts to create a name for a temporary file, which
will be a valid file name yet not exist when @code{tmpnam} checks for
it.  @var{s} must point to a buffer of at least @code{L_tmpnam} bytes,
or be @code{NULL}.  Use of this function creates a security risk, and it must
not be used in new projects.  Use @code{mkstemp} instead.

@end deftypefn

*/

#include <stdio.h>

#ifndef L_tmpnam
#define L_tmpnam 100
#endif
#ifndef P_tmpdir
#define P_tmpdir "/usr/tmp"
#endif

static char tmpnam_buffer[L_tmpnam];
static int tmpnam_counter;

extern int getpid (void);

char *
tmpnam (char *s)
{
  int pid = getpid ();

  if (s == NULL)
    s = tmpnam_buffer;

  /*  Generate the filename and make sure that there isn't one called
      it already.  */

  while (1)
    {
      FILE *f;
      sprintf (s, "%s/%s%x.%x", P_tmpdir, "t", pid, tmpnam_counter);
      f = fopen (s, "r");
      if (f == NULL)
	break;
      tmpnam_counter++;
      fclose (f);
    }

  return s;
}