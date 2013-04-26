
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

#include "common.h"

int kqfd;

void
test_kevent_signal_add(void)
{
    const char *test_id = "kevent(EVFILT_SIGNAL, EV_ADD)";
    struct kevent kev;

    test_begin(test_id);

    EV_SET(&kev, SIGUSR1, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    success();
}

void
test_kevent_signal_get(void)
{
    const char *test_id = "kevent(EVFILT_SIGNAL, wait)";
    struct kevent kev;

    test_begin(test_id);

    EV_SET(&kev, SIGUSR1, EVFILT_SIGNAL, EV_ADD, 0, 0, NULL);    
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    /* Block SIGUSR1, then send it to ourselves */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        err(1, "sigprocmask");
    if (kill(getpid(), SIGUSR1) < 0)
        err(1, "kill");

    kev.flags |= EV_CLEAR;
    kev.data = 1;
    kevent_cmp(&kev, kevent_get(kqfd));

    success();
}

void
test_kevent_signal_disable(void)
{
    const char *test_id = "kevent(EVFILT_SIGNAL, EV_DISABLE)";
    struct kevent kev;

    test_begin(test_id);

    EV_SET(&kev, SIGUSR1, EVFILT_SIGNAL, EV_DISABLE, 0, 0, NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    /* Block SIGUSR1, then send it to ourselves */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        err(1, "sigprocmask");
    if (kill(getpid(), SIGUSR1) < 0)
        err(1, "kill");

    test_no_kevents();

    success();
}

void
test_kevent_signal_enable(void)
{
    const char *test_id = "kevent(EVFILT_SIGNAL, EV_ENABLE)";
    struct kevent kev;

    test_begin(test_id);

    EV_SET(&kev, SIGUSR1, EVFILT_SIGNAL, EV_ENABLE, 0, 0, NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    /* Block SIGUSR1, then send it to ourselves */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        err(1, "sigprocmask");
    if (kill(getpid(), SIGUSR1) < 0)
        err(1, "kill");

    kev.flags = EV_ADD | EV_CLEAR;
#if LIBKQUEUE
    kev.data = 1; /* WORKAROUND */
#else
    kev.data = 2; // one extra time from test_kevent_signal_disable()
#endif
    kevent_cmp(&kev, kevent_get(kqfd));

    /* Delete the watch */
    kev.flags = EV_DELETE;
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    success();
}

void
test_kevent_signal_del(void)
{
    const char *test_id = "kevent(EVFILT_SIGNAL, EV_DELETE)";
    struct kevent kev;

    test_begin(test_id);

    /* Delete the kevent */
    EV_SET(&kev, SIGUSR1, EVFILT_SIGNAL, EV_DELETE, 0, 0, NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    /* Block SIGUSR1, then send it to ourselves */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        err(1, "sigprocmask");
    if (kill(getpid(), SIGUSR1) < 0)
        err(1, "kill");

    test_no_kevents();
    success();
}

void
test_kevent_signal_oneshot(void)
{
    const char *test_id = "kevent(EVFILT_SIGNAL, EV_ONESHOT)";
    struct kevent kev;

    test_begin(test_id);

    EV_SET(&kev, SIGUSR1, EVFILT_SIGNAL, EV_ADD | EV_ONESHOT, 0, 0, NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    /* Block SIGUSR1, then send it to ourselves */
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
        err(1, "sigprocmask");
    if (kill(getpid(), SIGUSR1) < 0)
        err(1, "kill");

    kev.flags |= EV_CLEAR;
    kev.data = 1;
    kevent_cmp(&kev, kevent_get(kqfd));

    /* Send another one and make sure we get no events */
    if (kill(getpid(), SIGUSR1) < 0)
        err(1, "kill");
    test_no_kevents();

    success();
}

void
test_evfilt_signal()
{
	kqfd = kqueue();
        test_kevent_signal_add();
        test_kevent_signal_del();
        test_kevent_signal_get();
        test_kevent_signal_disable();
        test_kevent_signal_enable();
        test_kevent_signal_oneshot();
	close(kqfd);
}