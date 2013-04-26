
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

/*
 * Timer device driver for power management events.
 * The code for suspend/resume is derived from APM device driver.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/syslog.h>
#include <machine/clock.h>

#include <isa/isavar.h>

static devclass_t pmtimer_devclass;

/* reject any PnP devices for now */
static struct isa_pnp_id pmtimer_ids[] = {
	{0}
};

static void
pmtimer_identify(driver_t *driver, device_t parent)
{
	device_t child;

	/*
	 * Only add a child if one doesn't exist already.
	 */
	child = devclass_get_device(pmtimer_devclass, 0);
	if (child == NULL) {
		child = BUS_ADD_CHILD(parent, 0, "pmtimer", 0);
		if (child == NULL)
			panic("pmtimer_identify");
	}
}

static int
pmtimer_probe(device_t dev)
{

	if (ISA_PNP_PROBE(device_get_parent(dev), dev, pmtimer_ids) == ENXIO)
		return (ENXIO);

	/* only one instance always */
	return (device_get_unit(dev));
}

static struct timeval suspend_time;
static struct timeval diff_time;

static int
pmtimer_suspend(device_t dev)
{

	microtime(&diff_time);
	inittodr(0);
	microtime(&suspend_time);
	timevalsub(&diff_time, &suspend_time);
	return (0);
}

static int
pmtimer_resume(device_t dev)
{
	u_int second, minute, hour;
	struct timeval resume_time, tmp_time;

	/* modified for adjkerntz */
	timer_restore();		/* restore the all timers */
	inittodr(0);			/* adjust time to RTC */
	microtime(&resume_time);
	getmicrotime(&tmp_time);
	timevaladd(&tmp_time, &diff_time);

#ifdef FIXME
	/* XXX THIS DOESN'T WORK!!! */
	time = tmp_time;
#endif

#ifdef PMTIMER_FIXUP_CALLTODO
	/* Calculate the delta time suspended */
	timevalsub(&resume_time, &suspend_time);
	/* Fixup the calltodo list with the delta time. */
	adjust_timeout_calltodo(&resume_time);
	/* 
	 * We've already calculated resume_time to be the delta between 
	 * the suspend and the resume. 
	 */
	second = resume_time.tv_sec; 
#else /* !PMTIMER_FIXUP_CALLTODO */
	second = resume_time.tv_sec - suspend_time.tv_sec; 
#endif /* PMTIMER_FIXUP_CALLTODO */
	hour = second / 3600;
	second %= 3600;
	minute = second / 60;
	second %= 60;
	log(LOG_NOTICE, "wakeup from sleeping state (slept %02d:%02d:%02d)\n",
		hour, minute, second);
	return (0);
}

static device_method_t pmtimer_methods[] = {
	/* Device interface */
	DEVMETHOD(device_identify,	pmtimer_identify),
	DEVMETHOD(device_probe,		pmtimer_probe),
	DEVMETHOD(device_attach,	bus_generic_attach),
	DEVMETHOD(device_suspend,	pmtimer_suspend),
	DEVMETHOD(device_resume,	pmtimer_resume),
	{ 0, 0 }
};

static driver_t pmtimer_driver = {
	"pmtimer",
	pmtimer_methods,
	1,		/* no softc */
};

DRIVER_MODULE(pmtimer, isa, pmtimer_driver, pmtimer_devclass, 0, 0);