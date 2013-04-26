
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

#include <sys/types.h>

#include <signal.h>

#include "log.h"
#include "sig.h"

static int caused[NSIG];	/* An array of pending signals */
static int necessary;		/* Anything set ? */
static sig_type handler[NSIG];	/* all start at SIG_DFL */


/*
 * Record a signal in the "caused" array
 *
 * This function is the only thing actually called in signal context.  It
 * records that a signal has been caused and that sig_Handle() should be
 * called (in non-signal context) as soon as possible to process that
 * signal.
 */
static void
signal_recorder(int sig)
{
  caused[sig - 1]++;
  necessary = 1;
}


/*
 * Set up signal_recorder to handle the given sig and record ``fn'' as
 * the function to ultimately call in sig_Handle().  ``fn'' will not be
 * called in signal context (as sig_Handle() is not called in signal
 * context).
 */
sig_type
sig_signal(int sig, sig_type fn)
{
  sig_type Result;

  if (sig <= 0 || sig > NSIG) {
    /* Oops - we must be a bit out of date (too many sigs ?) */
    log_Printf(LogALERT, "Eeek! %s:%d: I must be out of date!\n",
	      __FILE__, __LINE__);
    return signal(sig, fn);
  }
  Result = handler[sig - 1];
  if (fn == SIG_DFL || fn == SIG_IGN) {
    signal(sig, fn);
    handler[sig - 1] = (sig_type) 0;
  } else {
    handler[sig - 1] = fn;
    signal(sig, signal_recorder);
  }
  caused[sig - 1] = 0;
  return Result;
}


/*
 * Call the handlers for any pending signals
 *
 * This function is called from a non-signal context - in fact, it's
 * called every time select() in DoLoop() returns - just in case
 * select() returned due to a signal being recorded by signal_recorder().
 */
int
sig_Handle()
{
  int sig;
  int got;
  int result;

  result = 0;
  if (necessary) {
    /* We've *probably* got something in `caused' set */
    necessary = 0;
    /* `necessary' might go back to 1 while we're in here.... */
    do {
      got = 0;
      for (sig = 0; sig < NSIG; sig++)
        if (caused[sig]) {
	  caused[sig]--;
	  got++;
	  result++;
	  (*handler[sig])(sig + 1);
        }
    } while (got);
  }

  return result;
}