
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
#include <sys/param.h>
#include <sys/bus.h>
#include <sys/time.h>
#include <sys/clock.h>
#include <sys/resource.h>
#include <sys/systm.h>
#include <sys/rman.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <machine/bus.h>

#include <arm/s3c2xx0/s3c24x0reg.h>

#include "clock_if.h"

#define YEAR_BASE		2000

struct s3c2xx0_rtc_softc {
	struct resource *mem_res;
};

static int
s3c2xx0_rtc_probe(device_t dev)
{

	device_set_desc(dev, "Samsung Integrated RTC");
	return (0);
}

static int
s3c2xx0_rtc_attach(device_t dev)
{
	struct s3c2xx0_rtc_softc *sc;
	int error, rid;

	sc = device_get_softc(dev);
 	error = 0;

	rid = 0;
	sc->mem_res = bus_alloc_resource_any(dev, SYS_RES_IOPORT, &rid,
	    RF_ACTIVE);
	if (sc->mem_res == NULL) {
		error = ENOMEM;
		goto out;
	}

	bus_write_1(sc->mem_res, RTC_RTCCON, RTCCON_RTCEN);
	clock_register(dev, 1000000);

out:
	return (error);
}

static int
s3c2xx0_rtc_gettime(device_t dev, struct timespec *ts)
{
	struct s3c2xx0_rtc_softc *sc;
	struct clocktime ct;

#define READ_TIME() do {						\
	ct.year = YEAR_BASE + FROMBCD(bus_read_1(sc->mem_res, RTC_BCDYEAR)); \
	ct.mon = FROMBCD(bus_read_1(sc->mem_res, RTC_BCDMON));		\
	ct.dow = FROMBCD(bus_read_1(sc->mem_res, RTC_BCDDAY));		\
	ct.day = FROMBCD(bus_read_1(sc->mem_res, RTC_BCDDATE));		\
	ct.hour = FROMBCD(bus_read_1(sc->mem_res, RTC_BCDHOUR));	\
	ct.min = FROMBCD(bus_read_1(sc->mem_res, RTC_BCDMIN));		\
	ct.sec = FROMBCD(bus_read_1(sc->mem_res, RTC_BCDSEC));		\
} while (0)

	sc = device_get_softc(dev);

	ct.nsec = 0;
	READ_TIME();
	/*
	 * Check if we could have read incorrect values
	 * as the values could have changed.
	 */
	if (ct.sec == 0) {
		READ_TIME();
	}

	ct.dow = -1;

#undef READ_TIME
	return (clock_ct_to_ts(&ct, ts));
}

static int
s3c2xx0_rtc_settime(device_t dev, struct timespec *ts)
{
	struct s3c2xx0_rtc_softc *sc;
	struct clocktime ct;

	sc = device_get_softc(dev);

	/* Resolution: 1 sec */
	if (ts->tv_nsec >= 500000000)
		ts->tv_sec++;
	ts->tv_nsec = 0;
	clock_ts_to_ct(ts, &ct);

	bus_write_1(sc->mem_res, RTC_BCDSEC, TOBCD(ct.sec));
	bus_write_1(sc->mem_res, RTC_BCDMIN, TOBCD(ct.min));
	bus_write_1(sc->mem_res, RTC_BCDHOUR, TOBCD(ct.hour));
	bus_write_1(sc->mem_res, RTC_BCDDATE, TOBCD(ct.day));
	bus_write_1(sc->mem_res, RTC_BCDDAY, TOBCD(ct.dow));
	bus_write_1(sc->mem_res, RTC_BCDMON, TOBCD(ct.mon));
	bus_write_1(sc->mem_res, RTC_BCDYEAR, TOBCD(ct.year - YEAR_BASE));

	return (0);
}

static device_method_t s3c2xx0_rtc_methods[] = {
	DEVMETHOD(device_probe,		s3c2xx0_rtc_probe),
	DEVMETHOD(device_attach,	s3c2xx0_rtc_attach),

	DEVMETHOD(clock_gettime,	s3c2xx0_rtc_gettime),
	DEVMETHOD(clock_settime,	s3c2xx0_rtc_settime),

	{ 0, 0 },
};

static driver_t s3c2xx0_rtc_driver = {
	"rtc",
	s3c2xx0_rtc_methods,
	sizeof(struct s3c2xx0_rtc_softc),
};
static devclass_t s3c2xx0_rtc_devclass;

DRIVER_MODULE(s3c2xx0_rtc, s3c24x0, s3c2xx0_rtc_driver, s3c2xx0_rtc_devclass,
    0, 0);