
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
/* Version of sigsetmask.c   Written by Steve Chamberlain (sac@cygnus.com).
   Contributed by Cygnus Support.
   This file is in the public doamin. */

/*

@deftypefn Supplemental int sigsetmask (int @var{set})

Sets the signal mask to the one provided in @var{set} and returns
the old mask (which, for libiberty's implementation, will always
be the value @code{1}).

@end deftypefn

*/

#define _POSIX_SOURCE
#include <ansidecl.h>
/* Including <sys/types.h> seems to be needed by ISC. */
#include <sys/types.h>
#include <signal.h>

extern void abort (void) ATTRIBUTE_NORETURN;

#ifdef SIG_SETMASK
int
sigsetmask (int set)
{
    sigset_t new_sig;
    sigset_t old_sig;
    
    sigemptyset (&new_sig);
    if (set != 0) {
      abort();	/* FIXME, we don't know how to translate old mask to new */
    }
    sigprocmask(SIG_SETMASK, &new_sig, &old_sig);
    return 1;	/* FIXME, we always return 1 as old value.  */
}
#endif