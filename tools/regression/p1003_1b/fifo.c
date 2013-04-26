
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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sched.h>
#include <signal.h>

volatile int ticked;
#define CAN_USE_ALARMS

#ifdef CAN_USE_ALARMS
void tick(int arg)
{
	ticked = 1;
}
#endif

/* Fifo: Verify that fifo and round-robin scheduling seem to work.
 *
 * This tests:
 * 1. That sched_rr_get_interval seems to work;
 * 2. That FIFO scheduling doesn't seeem to be round-robin;
 * 3. That round-robin scheduling seems to work.
 * 
 */
static pid_t child;
static void tidyup(void)
{
	if (child)
		kill(child, SIGHUP);
}

static double
tvsub(const struct timeval *a, const struct timeval *b)
{
	long sdiff;
	long udiff;

	sdiff = a->tv_sec - b->tv_sec;
	udiff = a->tv_usec - b->tv_usec;

	return (double)(sdiff * 1000000 + udiff) / 1e6;
}

int fifo(int argc, char *argv[])
{
	int e = 0;
	volatile long *p, pid;
	int i;
	struct sched_param fifo_param;
	struct timespec interval;
#define MAX_RANAT 32
	struct timeval ranat[MAX_RANAT];

#ifdef CAN_USE_ALARMS
	static struct itimerval itimerval;
#endif

	/* What is the round robin interval?
	 */

	if (sched_rr_get_interval(0, &interval) == -1) {
		perror("sched_rr_get_interval");
		exit(errno);
	}

#ifdef CAN_USE_ALARMS
	signal(SIGALRM, tick);
#endif

	fifo_param.sched_priority = 1;

	p = (long *)mmap(0, sizeof(*p),
	PROT_READ|PROT_WRITE, MAP_ANON|MAP_SHARED|MAP_INHERIT, -1, 0);

	if (p == (long *)-1)
		err(errno, "mmap");

	*p = 0;

	if (sched_setscheduler(0, SCHED_FIFO, &fifo_param) == -1)
	{
		perror("sched_setscheduler");
		return -1;
	}

	pid = getpid();

	if ((child = fork()) == 0)
	{
		/* Child process.  Just keep setting the pointer to our
		 * PID.  The parent will kill us when it wants to.
		 */

		pid = getpid();
		while (1)
			*p = pid;
	}
	else
	{
		atexit(tidyup);
		*p = pid;


		ticked = 0;

#ifdef CAN_USE_ALARMS
		/* Set an alarm for 250 times the round-robin interval.
		 * Then we will verify that a similar priority process
		 * will not run when we are using the FIFO scheduler.
		 */
		itimerval.it_value.tv_usec = interval.tv_nsec / (1000 / 250);

		itimerval.it_value.tv_sec = itimerval.it_value.tv_usec / 1000000;
		itimerval.it_value.tv_usec %= 1000000;


		if (setitimer(ITIMER_REAL, &itimerval, 0) == -1) {
			perror("setitimer");
			exit(errno);
		}
#endif


		gettimeofday(ranat, 0);
		i = 1;
		while (!ticked && i < MAX_RANAT)
			if (*p == child) {
				gettimeofday(ranat + i, 0);
				*p = 0;
				e = -1;
				i++;
			}

		if (e) {
			int j;

			fprintf(stderr,
			"SCHED_FIFO had erroneous context switches:\n");
			for (j = 1; j < i; j++) {
				fprintf(stderr, "%d %g\n", j,
					tvsub(ranat + j, ranat + j - 1));
			}
			return e;
		}

		/* Switch to the round robin scheduler and the child
		 * should run within twice the interval.
		 */
		if (sched_setscheduler(child, SCHED_RR, &fifo_param) == -1 ||
		sched_setscheduler(0, SCHED_RR, &fifo_param) == -1)
		{
			perror("sched_setscheduler");
			return -1;
		}

		e = -1;

		ticked = 0;

#ifdef CAN_USE_ALARMS

		/* Now we do want to see it run.  But only set
		 * the alarm for twice the interval:
		 */
		itimerval.it_value.tv_usec = interval.tv_nsec / 500;

		if (setitimer(ITIMER_REAL, &itimerval, 0) == -1) {
			perror("setitimer");
			exit(errno);
		}
#endif

		for (i = 0; !ticked; i++)
			if (*p == child) {
				e = 0;
				break;
			}

		if (e)
			fprintf(stderr,"Child never ran when it should have.\n");
	}

	exit(e);
}

#ifdef STANDALONE_TESTS
int main(int argc, char *argv[]) { return fifo(argc, argv); }
#endif