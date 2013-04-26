
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

#ifndef lint
static const char rcsid[] _U_ =
    "@(#) $Header: /tcpdump/master/tcpdump/setsignal.c,v 1.11 2003-11-16 09:36:42 guy Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <tcpdump-stdinc.h>

#include <signal.h>
#ifdef HAVE_SIGACTION
#include <string.h>
#endif

#ifdef HAVE_OS_PROTO_H
#include "os-proto.h"
#endif

#include "setsignal.h"

/*
 * An OS-independent signal() with, whenever possible, partial BSD
 * semantics, i.e. the signal handler is restored following service
 * of the signal, but system calls are *not* restarted, so that if
 * "pcap_breakloop()" is called in a signal handler in a live capture,
 * the read/recvfrom/whatever in the live capture doesn't get restarted,
 * it returns -1 and sets "errno" to EINTR, so we can break out of the
 * live capture loop.
 *
 * We use "sigaction()" if available.  We don't specify that the signal
 * should restart system calls, so that should always do what we want.
 *
 * Otherwise, if "sigset()" is available, it probably has BSD semantics
 * while "signal()" has traditional semantics, so we use "sigset()"; it
 * might cause system calls to be restarted for the signal, however.
 * I don't know whether, in any systems where it did cause system calls to
 * be restarted, there was a way to ask it not to do so; there may no
 * longer be any interesting systems without "sigaction()", however,
 * and, if there are, they might have "sigvec()" with SV_INTERRUPT
 * (which I think first appeared in 4.3BSD).
 *
 * Otherwise, we use "signal()" - which means we might get traditional
 * semantics, wherein system calls don't get restarted *but* the
 * signal handler is reset to SIG_DFL and the signal is not blocked,
 * so that a subsequent signal would kill the process immediately.
 *
 * Did I mention that signals suck?  At least in POSIX-compliant systems
 * they suck far less, as those systems have "sigaction()".
 */
RETSIGTYPE
(*setsignal (int sig, RETSIGTYPE (*func)(int)))(int)
{
#ifdef HAVE_SIGACTION
	struct sigaction old, new;

	memset(&new, 0, sizeof(new));
	new.sa_handler = func;
	if (sigaction(sig, &new, &old) < 0)
		return (SIG_ERR);
	return (old.sa_handler);

#else
#ifdef HAVE_SIGSET
	return (sigset(sig, func));
#else
	return (signal(sig, func));
#endif
#endif
}