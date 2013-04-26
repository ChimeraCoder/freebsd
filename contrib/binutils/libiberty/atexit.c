
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
/* Wrapper to implement ANSI C's atexit using SunOS's on_exit. */
/* This function is in the public domain.  --Mike Stump. */
/*

@deftypefn Supplemental int atexit (void (*@var{f})())

Causes function @var{f} to be called at exit.  Returns 0.

@end deftypefn

*/

#include "config.h"

#ifdef HAVE_ON_EXIT

int
atexit(void (*f)(void))
{
  /* If the system doesn't provide a definition for atexit, use on_exit
     if the system provides that.  */
  on_exit (f, 0);
  return 0;
}

#endif