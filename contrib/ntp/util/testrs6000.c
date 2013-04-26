
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
/* Checks for the RS/6000 AIX adjtime() bug, in which if a negative
 * offset is given, the system gets messed up and never completes the
 * adjustment.  If the problem is fixed, this program will print the
 * time, sit there for 10 seconds, and exit.  If the problem isn't fixed,
 * the program will print an occasional "result=nnnnnn" (the residual
 * slew from adjtime()).
 *
 * Compile this with bsdcc and run it as root!
 */#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <stdio.h>

int timeout();
struct timeval adjustment, result;

int
main (
	int argc,
	char *argv[]
	)
{
	struct itimerval value, oldvalue;
	int i;
	time_t curtime;

	curtime = time(0);
	printf("Starting: %s", ctime(&curtime));
	value.it_interval.tv_sec = value.it_value.tv_sec = 1;
	value.it_interval.tv_usec = value.it_value.tv_usec = 0;
	adjustment.tv_sec = 0;
	adjustment.tv_usec = -2000;
	signal(SIGALRM, timeout);
	setitimer(ITIMER_REAL, &value, &oldvalue);
	for (i=0; i<10; i++) {
		pause();
	}
}

int
timeout(
	int sig,
	int code,
	struct sigcontext *scp
	)
{
	signal (SIGALRM, timeout);
	if (adjtime(&adjustment, &result)) 
	    printf("adjtime call failed\n");
	if (result.tv_sec != 0 || result.tv_usec != 0) {
		printf("result.u = %d.%06.6d  ", (int) result.tv_sec,
		       (int) result.tv_usec);
	}
}