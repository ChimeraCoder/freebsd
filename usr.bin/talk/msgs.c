
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

#include <sys/cdefs.h>

__FBSDID("$FreeBSD$");

#ifndef lint
static const char sccsid[] = "@(#)msgs.c	8.1 (Berkeley) 6/6/93";
#endif

/*
 * A package to display what is happening every MSG_INTERVAL seconds
 * if we are slow connecting.
 */

#include <signal.h>

#include "talk.h"

#define MSG_INTERVAL 4

const char	*current_state;
int	current_line = 0;

/* ARGSUSED */
void
disp_msg(int signo __unused)
{
	message(current_state);
}

void
start_msgs(void)
{
	struct itimerval itimer;

	message(current_state);
	signal(SIGALRM, disp_msg);
	itimer.it_value.tv_sec = itimer.it_interval.tv_sec = MSG_INTERVAL;
	itimer.it_value.tv_usec = itimer.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &itimer, (struct itimerval *)0);
}

void
end_msgs(void)
{
	struct itimerval itimer;

	timerclear(&itimer.it_value);
	timerclear(&itimer.it_interval);
	setitimer(ITIMER_REAL, &itimer, (struct itimerval *)0);
	signal(SIGALRM, SIG_DFL);
}