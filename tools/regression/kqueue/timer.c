
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
test_kevent_timer_add(void)
{
    const char *test_id = "kevent(EVFILT_TIMER, EV_ADD)";
    struct kevent kev;

    test_begin(test_id);

    EV_SET(&kev, 1, EVFILT_TIMER, EV_ADD, 0, 1000, NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    success();
}

void
test_kevent_timer_del(void)
{
    const char *test_id = "kevent(EVFILT_TIMER, EV_DELETE)";
    struct kevent kev;

    test_begin(test_id);

    EV_SET(&kev, 1, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    test_no_kevents();

    success();
}

void
test_kevent_timer_get(void)
{
    const char *test_id = "kevent(EVFILT_TIMER, wait)";
    struct kevent kev;

    test_begin(test_id);

    EV_SET(&kev, 1, EVFILT_TIMER, EV_ADD, 0, 1000, NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    kev.flags |= EV_CLEAR;
    kev.data = 1; 
    kevent_cmp(&kev, kevent_get(kqfd));

    EV_SET(&kev, 1, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    success();
}

static void
test_oneshot(void)
{
    const char *test_id = "kevent(EVFILT_TIMER, EV_ONESHOT)";
    struct kevent kev;

    test_begin(test_id);

    test_no_kevents();

    EV_SET(&kev, vnode_fd, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, 500,NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    /* Retrieve the event */
    kev.flags = EV_ADD | EV_CLEAR | EV_ONESHOT;
    kev.data = 1; 
    kevent_cmp(&kev, kevent_get(kqfd));

    /* Check if the event occurs again */
    sleep(3);
    test_no_kevents();


    success();
}

static void
test_periodic(void)
{
    const char *test_id = "kevent(EVFILT_TIMER, periodic)";
    struct kevent kev;

    test_begin(test_id);

    test_no_kevents();

    EV_SET(&kev, vnode_fd, EVFILT_TIMER, EV_ADD, 0, 1000,NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    /* Retrieve the event */
    kev.flags = EV_ADD | EV_CLEAR;
    kev.data = 1; 
    kevent_cmp(&kev, kevent_get(kqfd));

    /* Check if the event occurs again */
    sleep(1);
    kevent_cmp(&kev, kevent_get(kqfd));

    /* Delete the event */
    kev.flags = EV_DELETE;
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    success();
}

static void
disable_and_enable(void)
{
    const char *test_id = "kevent(EVFILT_TIMER, EV_DISABLE and EV_ENABLE)";
    struct kevent kev;

    test_begin(test_id);

    test_no_kevents();

    /* Add the watch and immediately disable it */
    EV_SET(&kev, vnode_fd, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, 2000,NULL);
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);
    kev.flags = EV_DISABLE;
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);
    test_no_kevents();

    /* Re-enable and check again */
    kev.flags = EV_ENABLE;
    if (kevent(kqfd, &kev, 1, NULL, 0, NULL) < 0)
        err(1, "%s", test_id);

    kev.flags = EV_ADD | EV_CLEAR | EV_ONESHOT;
    kev.data = 1; 
    kevent_cmp(&kev, kevent_get(kqfd));

    success();
}

void
test_evfilt_timer()
{
	kqfd = kqueue();
    test_kevent_timer_add();
    test_kevent_timer_del();
    test_kevent_timer_get();
    test_oneshot();
    test_periodic();
    disable_and_enable();
	close(kqfd);
}