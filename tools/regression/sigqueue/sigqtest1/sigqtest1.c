
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
/* $FreeBSD$ */#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int received;

void
handler(int sig, siginfo_t *si, void *ctx)
{
	if (si->si_code != SI_QUEUE)
		errx(1, "si_code != SI_QUEUE");
	if (si->si_value.sival_int != received)
		errx(1, "signal is out of order");
	received++;
}

int
main()
{
	struct sigaction sa;
	union sigval val;
	int ret;
	int i;
	sigset_t set;

	sa.sa_flags = SA_SIGINFO;
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = handler;
	sigaction(SIGRTMIN, &sa, NULL);
	sigemptyset(&set);
	sigaddset(&set, SIGRTMIN);
	sigprocmask(SIG_BLOCK, &set, NULL);
	i = 0;
	for (;;) {
		val.sival_int = i;
		ret = sigqueue(getpid(), SIGRTMIN, val);
		if (ret == -1) {
			if (errno != EAGAIN) {
				errx(1, "errno != EAGAIN");
			}
			break;
		}
		i++;
	}
	sigprocmask(SIG_UNBLOCK, &set, NULL);
	if (received != i)
		errx(1, "error, signal lost");
	printf("OK\n");
}