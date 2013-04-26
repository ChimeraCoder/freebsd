
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

#include "iprop.h"
RCSID("$Id$");

sig_atomic_t exit_flag;

static RETSIGTYPE
sigterm(int sig)
{
    exit_flag = sig;
}

void
setup_signal(void)
{
#ifdef HAVE_SIGACTION
    {
	struct sigaction sa;

	sa.sa_flags = 0;
	sa.sa_handler = sigterm;
	sigemptyset(&sa.sa_mask);

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGXCPU, &sa, NULL);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE, &sa, NULL);
    }
#else
    signal(SIGINT, sigterm);
    signal(SIGTERM, sigterm);
#ifndef NO_SIGXCPU
    signal(SIGXCPU, sigterm);
#endif
#ifndef NO_SIGPIPE
    signal(SIGPIPE, SIG_IGN);
#endif
#endif
}