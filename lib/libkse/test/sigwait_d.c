
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

static int		sigcounts[NSIG + 1];
static sigset_t		wait_mask;
static pthread_mutex_t	waiter_mutex;


static void *
sigwaiter (void *arg)
{
	int signo;
	sigset_t mask;

	/* Block SIGHUP */
	sigemptyset (&mask);
	sigaddset (&mask, SIGHUP);
	sigprocmask (SIG_BLOCK, &mask, NULL);

	while (sigcounts[SIGINT] == 0) {
		if (sigwait (&wait_mask, &signo) != 0) {
			fprintf (stderr,
			    "Unable to wait for signal, errno %d\n",
			    errno);
			exit (1);
		}
		sigcounts[signo]++;
		fprintf (stderr, "Sigwait caught signal %d\n", signo);

		/* Allow the main thread to prevent the sigwait. */
		pthread_mutex_lock (&waiter_mutex);
		pthread_mutex_unlock (&waiter_mutex);
	}

	pthread_exit (arg);
	return (NULL);
}


static void
sighandler (int signo)
{
	fprintf (stderr, "  -> Signal handler caught signal %d\n", signo);

	if ((signo >= 0) && (signo <= NSIG))
		sigcounts[signo]++;
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
	pthread_mutexattr_t mattr;
	pthread_attr_t	pattr;
	pthread_t	tid;
	void *		exit_status;
	struct sigaction act;

	/* Initialize our signal counts. */
	memset ((void *) sigcounts, 0, NSIG * sizeof (int));

	/* Setup our wait mask. */
	sigemptyset (&wait_mask);		/* Default action	*/
	sigaddset (&wait_mask, SIGHUP);		/* terminate		*/
	sigaddset (&wait_mask, SIGINT);		/* terminate		*/
	sigaddset (&wait_mask, SIGQUIT);	/* create core image	*/
	sigaddset (&wait_mask, SIGURG);		/* ignore		*/
	sigaddset (&wait_mask, SIGIO);		/* ignore		*/
	sigaddset (&wait_mask, SIGUSR1);	/* terminate		*/

	/* Ignore signals SIGHUP and SIGIO. */
	sigemptyset (&act.sa_mask);
	sigaddset (&act.sa_mask, SIGHUP);
	sigaddset (&act.sa_mask, SIGIO);
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	sigaction (SIGHUP, &act, NULL);
	sigaction (SIGIO, &act, NULL);

	/* Install a signal handler for SIGURG */
	sigemptyset (&act.sa_mask);
	sigaddset (&act.sa_mask, SIGURG);
	act.sa_handler = sighandler;
	act.sa_flags = SA_RESTART;
	sigaction (SIGURG, &act, NULL);

	/* Install a signal handler for SIGXCPU */
	sigemptyset (&act.sa_mask);
	sigaddset (&act.sa_mask, SIGXCPU);
	sigaction (SIGXCPU, &act, NULL);

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
	 * Initialize and create a mutex.
	 */
	if ((pthread_mutexattr_init (&mattr) != 0) ||
	    (pthread_mutex_init (&waiter_mutex, &mattr) != 0)) {
		fprintf (stderr, "Unable to create waiter mutex.\n");
		exit (1);
	}

	/*
	 * Create the sigwaiter thread.
	 */
	if (pthread_create (&tid, &pattr, sigwaiter, NULL) != 0) {
		fprintf (stderr, "Unable to create thread.\n");
		exit (1);
	}
#if defined(_LIBC_R_)
	pthread_set_name_np (tid, "sigwaiter");
#endif

	/*
	 * Verify that an ignored signal doesn't cause a wakeup.
	 * We don't have a handler installed for SIGIO.
	 */
	send_thread_signal (tid, SIGIO);
	sleep (1);
	send_process_signal (SIGIO);
	sleep (1);
	if (sigcounts[SIGIO] != 0)
		fprintf (stderr,
		    "FAIL: sigwait wakes up for ignored signal SIGIO.\n");

	/*
	 * Verify that a signal with a default action of ignore, for
	 * which we have a signal handler installed, will release a sigwait.
	 */
	send_thread_signal (tid, SIGURG);
	sleep (1);
	send_process_signal (SIGURG);
	sleep (1);
	if (sigcounts[SIGURG] != 2)
		fprintf (stderr, "FAIL: sigwait doesn't wake up for SIGURG.\n");

	/*
	 * Verify that a signal with a default action that terminates
	 * the process will release a sigwait.
	 */
	send_thread_signal (tid, SIGUSR1);
	sleep (1);
	send_process_signal (SIGUSR1);
	sleep (1);
	if (sigcounts[SIGUSR1] != 2)
		fprintf (stderr,
		    "FAIL: sigwait doesn't wake up for SIGUSR1.\n");

	/*
	 * Verify that if we install a signal handler for a previously
	 * ignored signal, an occurrence of this signal will release
	 * the (already waiting) sigwait.
	 */

	/* Install a signal handler for SIGHUP. */
	sigemptyset (&act.sa_mask);
	sigaddset (&act.sa_mask, SIGHUP);
	act.sa_handler = sighandler;
	act.sa_flags = SA_RESTART;
	sigaction (SIGHUP, &act, NULL);

	/* Sending SIGHUP should release the sigwait. */
	send_process_signal (SIGHUP);
	sleep (1);
	send_thread_signal (tid, SIGHUP);
	sleep (1);
	if (sigcounts[SIGHUP] != 2)
		fprintf (stderr, "FAIL: sigwait doesn't wake up for SIGHUP.\n");

	/*
	 * Verify that a pending signal in the waiters mask will
	 * cause sigwait to return the pending signal.  We do this
	 * by taking the waiters mutex and signaling the waiter to
	 * release him from the sigwait.  The waiter will block
	 * on taking the mutex, and we can then send the waiter a
	 * signal which should be added to his pending signals.
	 * The next time the waiter does a sigwait, he should
	 * return with the pending signal.
	 */
	sigcounts[SIGHUP] = 0;
 	pthread_mutex_lock (&waiter_mutex);
	/* Release the waiter from sigwait. */
	send_process_signal (SIGHUP);
	sleep (1);
	if (sigcounts[SIGHUP] != 1)
		fprintf (stderr, "FAIL: sigwait doesn't wake up for SIGHUP.\n");
	/*
	 * Add SIGHUP to the process pending signals.  Since there is
	 * a signal handler installed for SIGHUP and this signal is
	 * blocked from the waiter thread and unblocked in the main
	 * thread, the signal handler should be called once for SIGHUP.
	 */
	send_process_signal (SIGHUP);
	/* Release the waiter thread and allow him to run. */
	pthread_mutex_unlock (&waiter_mutex);
	sleep (1);
	if (sigcounts[SIGHUP] != 2)
		fprintf (stderr,
		    "FAIL: sigwait doesn't return for pending SIGHUP.\n");

	/*
	 * Repeat the above test using pthread_kill and SIGUSR1.
	 */
	sigcounts[SIGUSR1] = 0;
 	pthread_mutex_lock (&waiter_mutex);
	/* Release the waiter from sigwait. */
	send_thread_signal (tid, SIGUSR1);
	sleep (1);
	if (sigcounts[SIGUSR1] != 1)
		fprintf (stderr,
		    "FAIL: sigwait doesn't wake up for SIGUSR1.\n");
	/* Add SIGUSR1 to the waiters pending signals. */
	send_thread_signal (tid, SIGUSR1);
	/* Release the waiter thread and allow him to run. */
	pthread_mutex_unlock (&waiter_mutex);
	sleep (1);
	if (sigcounts[SIGUSR1] != 2)
		fprintf (stderr,
		    "FAIL: sigwait doesn't return for pending SIGUSR1.\n");

	/*
	 * Verify that we can still kill the process for a signal
	 * not being waited on by sigwait.
	 */
	send_process_signal (SIGPIPE);
	fprintf (stderr, "FAIL: SIGPIPE did not terminate process.\n");

	/*
	 * Wait for the thread to finish.
	 */
	pthread_join (tid, &exit_status);

	return (0);
}