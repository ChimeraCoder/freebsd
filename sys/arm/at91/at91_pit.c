
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
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/resource.h>
#include <sys/systm.h>
#include <sys/rman.h>
#include <sys/time.h>
#include <sys/timetc.h>
#include <sys/watchdog.h>

#include <machine/bus.h>
#include <machine/cpu.h>
#include <machine/cpufunc.h>
#include <machine/frame.h>
#include <machine/intr.h>
#include <machine/resource.h>

#include <arm/at91/at91var.h>
#include <arm/at91/at91_pitreg.h>

#ifndef PIT_PRESCALE
#define PIT_PRESCALE (16)
#endif

static struct pit_softc {
	struct resource	*mem_res;	/* Memory resource */
	void		*intrhand;	/* Interrupt handle */
	device_t	sc_dev;
} *sc;

static uint32_t timecount = 0;
static unsigned at91_pit_get_timecount(struct timecounter *tc);
static int pit_intr(void *arg);

static inline uint32_t
RD4(struct pit_softc *sc, bus_size_t off)
{

	return (bus_read_4(sc->mem_res, off));
}

static inline void
WR4(struct pit_softc *sc, bus_size_t off, uint32_t val)
{

	bus_write_4(sc->mem_res, off, val);
}

void
at91_pit_delay(int us)
{
	int32_t cnt, last, piv;
	uint64_t pit_freq;
	const uint64_t mhz  = 1E6;

	last = PIT_PIV(RD4(sc, PIT_PIIR));

	/* Max delay ~= 260s. @ 133Mhz */
	pit_freq = at91_master_clock / PIT_PRESCALE;
	cnt  = ((pit_freq * us) + (mhz -1)) / mhz;
	cnt  = (cnt <= 0) ? 1 : cnt;

	while (cnt > 0) {
		piv = PIT_PIV(RD4(sc, PIT_PIIR));
			cnt  -= piv - last ;
		if (piv < last)
			cnt -= PIT_PIV(~0u) - last;
		last = piv;
	}
}

static struct timecounter at91_pit_timecounter = {
	at91_pit_get_timecount, /* get_timecount */
	NULL, /* no poll_pps */
	0xffffffff, /* counter mask */
	0 / PIT_PRESCALE, /* frequency */
	"AT91SAM9 timer", /* name */
	1000 /* quality */
};

static int
at91_pit_probe(device_t dev)
{

	device_set_desc(dev, "AT91SAM9 PIT");
        return (0);
}

static int
at91_pit_attach(device_t dev)
{
	void *ih;
	int rid, err = 0;
	struct at91_softc *at91_sc;
	struct resource *irq;

	at91_sc = device_get_softc(device_get_parent(dev));
	sc = device_get_softc(dev);
	sc->sc_dev = dev;

	rid = 0;
	sc->mem_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &rid,
	    RF_ACTIVE);

	if (sc->mem_res == NULL)
		panic("couldn't allocate register resources");

	rid = 0;
	irq = bus_alloc_resource(dev, SYS_RES_IRQ, &rid, 1, 1, 1,
	    RF_ACTIVE | RF_SHAREABLE);
	if (!irq) {
		device_printf(dev, "could not allocate interrupt resources.\n");
		err = ENOMEM;
		goto out;
	}

	/* Activate the interrupt. */
	err = bus_setup_intr(dev, irq, INTR_TYPE_CLK, pit_intr, NULL, NULL,
	    &ih);

	at91_pit_timecounter.tc_frequency =  at91_master_clock / PIT_PRESCALE;
	tc_init(&at91_pit_timecounter);

	/* Enable the PIT here. */
	WR4(sc, PIT_MR, PIT_PIV(at91_master_clock / PIT_PRESCALE / hz) |
	    PIT_EN | PIT_IEN);
out:
	return (err);
}

static device_method_t at91_pit_methods[] = {
	DEVMETHOD(device_probe, at91_pit_probe),
	DEVMETHOD(device_attach, at91_pit_attach),
	DEVMETHOD_END
};

static driver_t at91_pit_driver = {
	"at91_pit",
	at91_pit_methods,
	sizeof(struct pit_softc),
};

static devclass_t at91_pit_devclass;

DRIVER_MODULE(at91_pit, atmelarm, at91_pit_driver, at91_pit_devclass, NULL,
    NULL);

static int
pit_intr(void *arg)
{
	struct trapframe *fp = arg;
	uint32_t icnt;

	if (RD4(sc, PIT_SR) & PIT_PITS_DONE) {
		icnt = RD4(sc, PIT_PIVR) >> 20;

		/* Just add in the overflows we just read */
		timecount +=  PIT_PIV(RD4(sc, PIT_MR)) * icnt;

		hardclock(TRAPF_USERMODE(fp), TRAPF_PC(fp));
		return (FILTER_HANDLED);
	}
	return (FILTER_STRAY);
}

static unsigned
at91_pit_get_timecount(struct timecounter *tc)
{
	uint32_t piir, icnt;

	piir = RD4(sc, PIT_PIIR); /* Current  count | over flows */
	icnt = piir >> 20;	/* Overflows */
	return (timecount + PIT_PIV(piir) + PIT_PIV(RD4(sc, PIT_MR)) * icnt);
}