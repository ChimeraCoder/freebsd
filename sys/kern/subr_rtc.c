
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

/*
 * Helpers for time-of-day clocks. This is useful for architectures that need
 * support multiple models of such clocks, and generally serves to make the
 * code more machine-independent.
 * If the clock in question can also be used as a time counter, the driver
 * needs to initiate this.
 * This code is not yet used by all architectures.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_ffclock.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/bus.h>
#include <sys/clock.h>
#include <sys/sysctl.h>
#ifdef FFCLOCK
#include <sys/timeffc.h>
#endif
#include <sys/timetc.h>

#include "clock_if.h"

static device_t clock_dev = NULL;
static long clock_res;
static struct timespec clock_adj;

/* XXX: should be kern. now, it's no longer machdep.  */
static int disable_rtc_set;
SYSCTL_INT(_machdep, OID_AUTO, disable_rtc_set, CTLFLAG_RW, &disable_rtc_set,
    0, "Disallow adjusting time-of-day clock");

void
clock_register(device_t dev, long res)	/* res has units of microseconds */
{

	if (clock_dev != NULL) {
		if (clock_res > res) {
			if (bootverbose)
				device_printf(dev, "not installed as "
				    "time-of-day clock: clock %s has higher "
				    "resolution\n", device_get_name(clock_dev));
			return;
		}
		if (bootverbose)
			device_printf(clock_dev, "removed as "
			    "time-of-day clock: clock %s has higher "
			    "resolution\n", device_get_name(dev));
	}
	clock_dev = dev;
	clock_res = res;
	clock_adj.tv_sec = res / 2 / 1000000;
	clock_adj.tv_nsec = res / 2 % 1000000 * 1000;
	if (bootverbose)
		device_printf(dev, "registered as a time-of-day clock "
		    "(resolution %ldus, adjustment %jd.%09jds)\n", res,
		    (intmax_t)clock_adj.tv_sec, (intmax_t)clock_adj.tv_nsec);
}

/*
 * inittodr and settodr derived from the i386 versions written
 * by Christoph Robitschko <chmr@edvz.tu-graz.ac.at>,  reintroduced and
 * updated by Chris Stenton <chris@gnome.co.uk> 8/10/94
 */

/*
 * Initialize the time of day register, based on the time base which is, e.g.
 * from a filesystem.
 */
void
inittodr(time_t base)
{
	struct timespec ts;
	int error;

	if (clock_dev == NULL) {
		printf("warning: no time-of-day clock registered, system time "
		    "will not be set accurately\n");
		goto wrong_time;
	}
	/* XXX: We should poll all registered RTCs in case of failure */
	error = CLOCK_GETTIME(clock_dev, &ts);
	if (error != 0 && error != EINVAL) {
		printf("warning: clock_gettime failed (%d), the system time "
		    "will not be set accurately\n", error);
		goto wrong_time;
	}
	if (error == EINVAL || ts.tv_sec < 0) {
		printf("Invalid time in real time clock.\n"
		    "Check and reset the date immediately!\n");
		goto wrong_time;
	}

	ts.tv_sec += utc_offset();
	timespecadd(&ts, &clock_adj);
	tc_setclock(&ts);
#ifdef FFCLOCK
	ffclock_reset_clock(&ts);
#endif
	return;

wrong_time:
	if (base > 0) {
		ts.tv_sec = base;
		ts.tv_nsec = 0;
		tc_setclock(&ts);
	}
}

/*
 * Write system time back to RTC
 */
void
resettodr(void)
{
	struct timespec ts;
	int error;

	if (disable_rtc_set || clock_dev == NULL)
		return;

	getnanotime(&ts);
	timespecadd(&ts, &clock_adj);
	ts.tv_sec -= utc_offset();
	/* XXX: We should really set all registered RTCs */
	if ((error = CLOCK_SETTIME(clock_dev, &ts)) != 0)
		printf("warning: clock_settime failed (%d), time-of-day clock "
		    "not adjusted to system time\n", error);
}