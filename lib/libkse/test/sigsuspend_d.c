
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
#include <stdlib.h>
#include <unistd.h>

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#if defined(_LIBC_R_)
#include <pthread_np.h>
#endif

static int	sigcounts[NSIG + 1];
static int	sigfifo[NSIG + 1];
static int	fifo_depth = 0;
static sigset_t suspender_mask;
static pthread_t suspender_tid;


static void *
sigsuspender (void *arg)
{
	int save_count, status, i;
	sigset_t run_mask;

	/* Run with all signals blocked. */
	sigfillset (&run_mask);
	sigprocmask (SIG_SETMASK, &run_mask, NULL);

	/* Allow these signals to wake us up during a sigsuspend. */
	sigfillset (&suspender_mask);		/* Default action	*/
	sigdelset (&suspender_mask, SIGKILL);	/* Cannot catch		*/
	sigdelset (&suspender_mask, SIGSTOP);	/* Cannot catch		*/
	sigdelset (&suspender_mask, SIGINT);	/* terminate		*/
	sigdelset (&suspender_mask, SIGHUP);	/* terminate		*/
	sigdelset (&suspender_mask, SIGQUIT);	/* create core image	*/
	sigdelset (&suspender_mask, SIGURG);	/* ignore		*/
	sigdelset (&suspender_mask, SIGIO);	/* ignore		*/
	sigdelset (&suspender_mask, SIGUSR2);	/* terminate		*/

	while (sigcounts[SIGINT] == 0) {
		save_count = sigcounts[SIGUSR2];

		status = sigsuspend (&suspender_mask);
		if ((status == 0) || (errno != EINTR)) {
			fprintf (stderr, "Unable to suspend for signals, "
				"errno %d, return value %d\n",
				errno, status);
			exit (1);
		}
		for (i = 0; i < fifo_depth; i++)
			fprintf (stderr, "Sigsuspend woke up by signal %d\n",
				sigfifo[i]);
		fifo_depth = 0;
	}

	pthread_exit (arg);
	return (NULL);
}


static void
sighandler (int signo)
{
	sigset_t set, suspend_set;
	pthread_t self;

	if ((signo >= 0) && (signo <= NSIG))
		sigcounts[signo]++;

	/*
	 * If we are running on behalf of the suspender thread,
	 * ensure that we have the correct mask set.
	 */
	self = pthread_self ();
	if (self == suspender_tid) {
		sigfifo[fifo_depth] = signo;
		fifo_depth++;
		fprintf (stderr,
		    "  -> Suspender thread signal handler caught signal %d\n",
		    signo);

		/* Get the current signal mask. */
		sigprocmask (SIG_SETMASK, NULL, &set);

		/* The handler should run with the current signal masked. */
		suspend_set = suspender_mask;
		sigaddset(&suspend_set, signo);

		if (memcmp(&set, &suspend_set, sizeof(set)))
			fprintf (stderr,
			    "  >>> FAIL: sigsuspender signal handler running "
			    "with incorrect mask.\n");
	}
	else
		fprintf (stderr,
		    "  -> Main thread signal handler caught signal %d\n",
		    signo);
}


static void
send_thread_signal (pthread_t tid, int signo)
{
	if (pthread_kill (tid, signo) != 0) {
		fprintf (stderr, "Unable to send thread signal, errno %d.\n",
		    errno);
		exit (1);
	}
}


static void
send_process_signal (int signo)
{
	if (kill (getpid (), signo) != 0) {
		fprintf (stderr, "Unable to send process signal, errno %d.\n",
		    errno);
		exit (1);
	}
}


int main (int argc, char *argv[])
{
	pthread_attr_t	pattr;
	void *		exit_status;
	struct sigaction act;
	sigset_t	oldset;
	sigset_t	newset;

	/* Initialize our signal counts. */
	memset ((void *) sigcounts, 0, NSIG * sizeof (int));

	/* Ignore signal SIGIO. */
	sigemptyset (&act.sa_mask);
	sigaddset (&act.sa_mask, SIGIO);
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	sigaction (SIGIO, &act, NULL);

	/* Install a signal handler for SIGURG. */
	sigemptyset (&act.sa_mask);
	sigaddset (&act.sa_mask, SIGURG);
	act.sa_handler = sighandler;
	act.sa_flags = SA_RESTART;
	sigaction (SIGURG, &act, NULL);

	/* Install a signal handler for SIGXCPU */
	sigemptyset (&act.sa_mask);
	sigaddset (&act.sa_mask, SIGXCPU);
	sigaction (SIGXCPU, &act, NULL);

	/* Get our current signal mask. */
	sigprocmask (SIG_SETMASK, NULL, &oldset);

	/* Mask out SIGUSR1 and SIGUSR2. */
	newset = oldset;
	sigaddset (&newset, SIGUSR1);
	sigaddset (&newset, SIGUSR2);
	sigprocmask (SIG_SETMASK, &newset, NULL);

	/* Install a signal handler for SIGUSR1 */
	sigemptyset (&act.sa_mask);
	sigaddset (&act.sa_mask, SIGUSR1);
	sigaction (SIGUSR1, &act, NULL);

	/* Install a signal handler for SIGUSR2 */
	sigemptyset (&act.sa_mask);
	sigaddset (&act.sa_mask, SIGUSR2);
	sigaction (SIGUSR2, &act, NULL);

	/*
	 * Initialize the thread attribute.
	 */
	if ((pthread_attr_init (&pattr) != 0) ||
	    (pthread_attr_setdetachstate (&pattr,
	    PTHREAD_CREATE_JOINABLE) != 0)) {
		fprintf (stderr, "Unable to initialize thread attributes.\n");
		exit (1);
	}

	/*
	 * Create the sigsuspender thread.
	 */
	if (pthread_create (&suspender_tid, &pattr, sigsuspender, NULL) != 0) {
		fprintf (stderr, "Unable to create thread, errno %d.\n", errno);
		exit (1);
	}
#if defined(_LIBC_R_)
	pthread_set_name_np (suspender_tid, "sigsuspender");
#endif

	/*
	 * Verify that an ignored signal doesn't cause a wakeup.
	 * We don't have a handler installed for SIGIO.
	 */
	send_thread_signal (suspender_tid, SIGIO);
	sleep (1);
	send_process_signal (SIGIO);
	sleep (1);
	if (sigcounts[SIGIO] != 0)
		fprintf (stderr, "FAIL: sigsuspend wakes up for ignored signal "
			"SIGIO.\n");

	/*
	 * Verify that a signal with a default action of ignore, for
	 * which we have a signal handler installed, will release a
	 * sigsuspend.
	 */
	send_thread_signal (suspender_tid, SIGURG);
	sleep (1);
	send_process_signal (SIGURG);
	sleep (1);
	if (sigcounts[SIGURG] != 2)
		fprintf (stderr,
		    "FAIL: sigsuspend doesn't wake up for SIGURG.\n");

	/*
	 * Verify that a SIGUSR2 signal will release a sigsuspended
	 * thread.
	 */
	send_thread_signal (suspender_tid, SIGUSR2);
	sleep (1);
	send_process_signal (SIGUSR2);
	sleep (1);
	if (sigcounts[SIGUSR2] != 2)
		fprintf (stderr,
		    "FAIL: sigsuspend doesn't wake up for SIGUSR2.\n");

	/*
	 * Verify that a signal, blocked in both the main and
	 * sigsuspender threads, does not cause the signal handler
	 * to be called.
	 */
	send_thread_signal (suspender_tid, SIGUSR1);
	sleep (1);
	send_process_signal (SIGUSR1);
	sleep (1);
	if (sigcounts[SIGUSR1] != 0)
		fprintf (stderr, "FAIL: signal hander called for SIGUSR1.\n");

	/*
	 * Verify that we can still kill the process for a signal
	 * not being waited on by sigwait.
	 */
	send_process_signal (SIGPIPE);
	fprintf (stderr, "FAIL: SIGPIPE did not terminate process.\n");

	/*
	 * Wait for the thread to finish.
	 */
	pthread_join (suspender_tid, &exit_status);

	return (0);
}