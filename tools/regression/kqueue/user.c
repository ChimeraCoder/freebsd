
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

static void
add_and_delete(void)
{
    const char *test_id = "kevent(EVFILT_USER, EV_ADD and EV_DELETE)";
    struct kevent kev;

    test_begin(test_id);

    kevent_add(kqfd, &kev, 1, EVFILT_USER, EV_ADD, 0, 0, NULL);
    test_no_kevents();

    kevent_add(kqfd, &kev, 1, EVFILT_USER, EV_DELETE, 0, 0, NULL);
    test_no_kevents();

    success();
}

static void
event_wait(void)
{
    const char *test_id = "kevent(EVFILT_USER, wait)";
    struct kevent kev;

    test_begin(test_id);

    test_no_kevents();

    /* Add the event, and then trigger it */
    kevent_add(kqfd, &kev, 1, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, NULL);    
    kevent_add(kqfd, &kev, 1, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);    

    kev.fflags &= ~NOTE_FFCTRLMASK;
    kev.fflags &= ~NOTE_TRIGGER;
    kev.flags = EV_CLEAR;
    kevent_cmp(&kev, kevent_get(kqfd));

    test_no_kevents();

    success();
}

static void
disable_and_enable(void)
{
    const char *test_id = "kevent(EVFILT_USER, EV_DISABLE and EV_ENABLE)";
    struct kevent kev;

    test_begin(test_id);

    test_no_kevents();

    kevent_add(kqfd, &kev, 1, EVFILT_USER, EV_ADD, 0, 0, NULL); 
    kevent_add(kqfd, &kev, 1, EVFILT_USER, EV_DISABLE, 0, 0, NULL); 

    /* Trigger the event, but since it is disabled, nothing will happen. */
    kevent_add(kqfd, &kev, 1, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL); 
    test_no_kevents();

    kevent_add(kqfd, &kev, 1, EVFILT_USER, EV_ENABLE, 0, 0, NULL); 
    kevent_add(kqfd, &kev, 1, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL); 

    kev.flags = EV_CLEAR;
    kev.fflags &= ~NOTE_FFCTRLMASK;
    kev.fflags &= ~NOTE_TRIGGER;
    kevent_cmp(&kev, kevent_get(kqfd));

    success();
}

static void
oneshot(void)
{
    const char *test_id = "kevent(EVFILT_USER, EV_ONESHOT)";
    struct kevent kev;

    test_begin(test_id);

    test_no_kevents();

    kevent_add(kqfd, &kev, 2, EVFILT_USER, EV_ADD | EV_ONESHOT, 0, 0, NULL);

    puts("  -- event 1");
    kevent_add(kqfd, &kev, 2, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);    

    kev.flags = EV_ONESHOT;
    kev.fflags &= ~NOTE_FFCTRLMASK;
    kev.fflags &= ~NOTE_TRIGGER;
    kevent_cmp(&kev, kevent_get(kqfd));

    test_no_kevents();

    success();
}

void
test_evfilt_user()
{
	kqfd = kqueue();

    add_and_delete();
    event_wait();
    disable_and_enable();
    oneshot();
    /* TODO: try different fflags operations */

	close(kqfd);
}