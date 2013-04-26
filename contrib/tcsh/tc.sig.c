
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
#include "sh.h"

RCSID("$tcsh: tc.sig.c,v 3.40 2012/01/25 15:34:41 christos Exp $")

#include "tc.wait.h"

void
sigset_interrupting(int sig, void (*fn) (int))
{
    struct sigaction act;

    act.sa_handler = fn;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(sig, &act, NULL) == 0) {
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, sig);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
    }
}

static volatile sig_atomic_t alrmcatch_pending; /* = 0; */
static volatile sig_atomic_t pchild_pending; /* = 0; */
static volatile sig_atomic_t phup_pending; /* = 0; */
static volatile sig_atomic_t pintr_pending; /* = 0; */
int alrmcatch_disabled; /* = 0; */
int phup_disabled; /* = 0; */
int pchild_disabled; /* = 0; */
int pintr_disabled; /* = 0; */
int handle_interrupt; /* = 0; */

int
handle_pending_signals(void)
{
    int rv = 0;
    if (!phup_disabled && phup_pending) {
	phup_pending = 0;
	handle_interrupt++;
	phup();
	handle_interrupt--;
    }
    if (!pintr_disabled && pintr_pending) {
	pintr_pending = 0;
	handle_interrupt++;
	pintr();
	handle_interrupt--;
	rv = 1;
    }
    if (!pchild_disabled && pchild_pending) {
	pchild_pending = 0;
	handle_interrupt++;
	pchild();
	handle_interrupt--;
    }
    if (!alrmcatch_disabled && alrmcatch_pending) {
	alrmcatch_pending = 0;
	handle_interrupt++;
	alrmcatch();
	handle_interrupt--;
    }
    return rv;
}

void
queue_alrmcatch(int sig)
{
    USE(sig);
    alrmcatch_pending = 1;
}

void
queue_pchild(int sig)
{
    USE(sig);
    pchild_pending = 1;
}

void
queue_phup(int sig)
{
    USE(sig);
    phup_pending = 1;
}

void
queue_pintr(int sig)
{
    USE(sig);
    pintr_pending = 1;
}

void
disabled_cleanup(void *xdisabled)
{
    int *disabled;

    disabled = xdisabled;
    if (--*disabled == 0)
	handle_pending_signals();
}

void
pintr_disabled_restore(void *xold)
{
    int *old;

    old = xold;
    pintr_disabled = *old;
}

void
pintr_push_enable(int *saved)
{
    *saved = pintr_disabled;
    pintr_disabled = 0;
    cleanup_push(saved, pintr_disabled_restore);
    handle_pending_signals();
}