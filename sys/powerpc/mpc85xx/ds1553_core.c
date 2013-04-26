
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
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/clock.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <machine/bus.h>

#include <powerpc/mpc85xx/ds1553_reg.h>

static uint8_t ds1553_direct_read(device_t, bus_size_t);
static void ds1553_direct_write(device_t, bus_size_t, uint8_t);

int
ds1553_attach(device_t dev)
{
	struct ds1553_softc *sc;
	uint8_t sec, flags;

	sc = device_get_softc(dev);

	if (mtx_initialized(&sc->sc_mtx) == 0) {
		device_printf(dev, "%s: mutex not initialized\n", __func__);
		return (ENXIO);
	}

	if (sc->sc_read == NULL)
		sc->sc_read = ds1553_direct_read;
	if (sc->sc_write == NULL)
		sc->sc_write = ds1553_direct_write;

	sc->year_offset = POSIX_BASE_YEAR;

	mtx_lock_spin(&sc->sc_mtx);

	/* Turn RTC on if it was not on */
	sec = (*sc->sc_read)(dev, DS1553_OFF_SECONDS);
	if (sec & DS1553_BIT_OSC) {
		sec &= ~(DS1553_BIT_OSC);
		(*sc->sc_write)(dev, DS1553_OFF_SECONDS, sec);
	}

	/* Low-battery check */
	flags = (*sc->sc_read)(dev, DS1553_OFF_FLAGS);
	if (flags & DS1553_BIT_BLF)
		device_printf(dev, "voltage-low detected.\n");

	mtx_unlock_spin(&sc->sc_mtx);

	return (0);
}

/*
 * Get time of day and convert it to a struct timespec.
 * Return 0 on success, an error number otherwise.
 */
int
ds1553_gettime(device_t dev, struct timespec *ts)
{
	struct clocktime ct;
	struct ds1553_softc *sc;
	uint8_t control;

	sc = device_get_softc(dev);

	mtx_lock_spin(&sc->sc_mtx);

	control = (*sc->sc_read)(dev, DS1553_OFF_CONTROL) | DS1553_BIT_READ;
	(*sc->sc_write)(dev, DS1553_OFF_CONTROL, control);

	ct.nsec = 0;
	ct.sec = FROMBCD((*sc->sc_read)(dev, DS1553_OFF_SECONDS) &
	    DS1553_MASK_SECONDS);
	ct.min = FROMBCD((*sc->sc_read)(dev, DS1553_OFF_MINUTES) &
	    DS1553_MASK_MINUTES);
	ct.hour = FROMBCD((*sc->sc_read)(dev, DS1553_OFF_HOURS) &
	    DS1553_MASK_HOUR);
	ct.dow = FROMBCD((*sc->sc_read)(dev, DS1553_OFF_DAYOFWEEK) &
	    DS1553_MASK_DAYOFWEEK) - 1;
	ct.day = FROMBCD((*sc->sc_read)(dev, DS1553_OFF_DATE) &
	    DS1553_MASK_DATE);
	ct.mon = FROMBCD((*sc->sc_read)(dev, DS1553_OFF_MONTH) &
	    DS1553_MASK_MONTH);
	ct.year = FROMBCD((*sc->sc_read)(dev, DS1553_OFF_YEAR));

	control &= ~DS1553_BIT_READ;
	(*sc->sc_write)(dev, DS1553_OFF_CONTROL, control);

	ct.year += sc->year_offset;

	mtx_unlock_spin(&sc->sc_mtx);

	return (clock_ct_to_ts(&ct, ts));
}

/*
 * Set the time of day clock based on the value of the struct timespec arg.
 * Return 0 on success, an error number otherwise.
 */
int
ds1553_settime(device_t dev, struct timespec *ts)
{
	struct clocktime ct;
	struct ds1553_softc *sc;
	uint8_t control;

	sc = device_get_softc(dev);
	bzero(&ct, sizeof(struct clocktime));

	/* Accuracy is only one second. */
	if (ts->tv_nsec >= 500000000)
		ts->tv_sec++;
	ts->tv_nsec = 0;
	clock_ts_to_ct(ts, &ct);

	ct.year -= sc->year_offset;

	mtx_lock_spin(&sc->sc_mtx);

	/* Halt updates to external registers */
	control = (*sc->sc_read)(dev, DS1553_OFF_CONTROL) | DS1553_BIT_WRITE;
	(*sc->sc_write)(dev, DS1553_OFF_CONTROL, control);

	(*sc->sc_write)(dev, DS1553_OFF_SECONDS, TOBCD(ct.sec) &
	    DS1553_MASK_SECONDS);
	(*sc->sc_write)(dev, DS1553_OFF_MINUTES, TOBCD(ct.min) &
	    DS1553_MASK_MINUTES);
	(*sc->sc_write)(dev, DS1553_OFF_HOURS, TOBCD(ct.hour) &
	    DS1553_MASK_HOUR);
	(*sc->sc_write)(dev, DS1553_OFF_DAYOFWEEK, TOBCD(ct.dow + 1) &
	    DS1553_MASK_DAYOFWEEK);
	(*sc->sc_write)(dev, DS1553_OFF_DATE, TOBCD(ct.day) &
	    DS1553_MASK_DATE);
	(*sc->sc_write)(dev, DS1553_OFF_MONTH, TOBCD(ct.mon) &
	    DS1553_MASK_MONTH);
	(*sc->sc_write)(dev, DS1553_OFF_YEAR, TOBCD(ct.year));

	/* Resume updates to external registers */
	control &= ~DS1553_BIT_WRITE;
	(*sc->sc_write)(dev, DS1553_OFF_CONTROL, control);

	mtx_unlock_spin(&sc->sc_mtx);

	return (0);
}

static uint8_t
ds1553_direct_read(device_t dev, bus_size_t off)
{
	struct ds1553_softc *sc;

	sc = device_get_softc(dev);
	return (bus_space_read_1(sc->sc_bst, sc->sc_bsh, off));
}

static void
ds1553_direct_write(device_t dev, bus_size_t off, uint8_t val)
{
	struct ds1553_softc *sc;

	sc = device_get_softc(dev);
	bus_space_write_1(sc->sc_bst, sc->sc_bsh, off, val);
}