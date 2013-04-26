
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
 * Child driver for ChipCommon core.
 * This is not MI code at the moment.
 * Two 16C550 compatible UARTs live here. On the WGT634U, uart1 is the
 * system console, and uart0 is not pinned out.
 *  Because their presence is conditional, they should probably
 *  be attached from here.
 * GPIO lives here.
 * The hardware watchdog lives here.
 * Clock control registers live here.
 *  You don't need to read them to determine the clock speed on the 5365,
 *  which is always 200MHz and thus may be hardcoded (for now).
 * Flash config registers live here. There may or may not be system flash.
 * The external interface bus lives here (conditionally).
 * There is a JTAG interface here which may be used to attach probes to
 * the SoC for debugging.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/rman.h>
#include <sys/malloc.h>

#include <machine/bus.h>

#include <dev/siba/siba_ids.h>
#include <dev/siba/sibareg.h>
#include <dev/siba/sibavar.h>

static int	siba_cc_attach(device_t);
static int	siba_cc_probe(device_t);
static void	siba_cc_intr(void *v);

static int
siba_cc_probe(device_t dev)
{

	if (siba_get_vendor(dev) == SIBA_VID_BROADCOM &&
	    siba_get_device(dev) == SIBA_DEVID_CHIPCOMMON) {
		device_set_desc(dev, "ChipCommon core");
		return (BUS_PROBE_DEFAULT);
	}

	return (ENXIO);
}

struct siba_cc_softc {
	void *notused;
};

static int
siba_cc_attach(device_t dev)
{
	//struct siba_cc_softc *sc = device_get_softc(dev);
	struct resource *mem;
	struct resource *irq;
	int rid;

	/*
	 * Allocate the resources which the parent bus has already
	 * determined for us.
	 * TODO: interrupt routing
	 */
#define MIPS_MEM_RID 0x20
	rid = MIPS_MEM_RID;
	mem = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &rid, RF_ACTIVE);
	if (mem == NULL) {
		device_printf(dev, "unable to allocate memory\n");
		return (ENXIO);
	}

	rid = 0;
	irq = bus_alloc_resource_any(dev, SYS_RES_IRQ, &rid, 0);
	if (irq == NULL) {
		device_printf(dev, "unable to allocate irq\n");
		return (ENXIO);
	}

	/* now setup the interrupt */
	/* may be fast, exclusive or mpsafe at a later date */

	/*
	 * XXX is this interrupt line in ChipCommon used for anything
	 * other than the uart? in that case we shouldn't hog it ourselves
	 * and let uart claim it to avoid polled mode.
	 */
	int err;
	void *cookie;
	err = bus_setup_intr(dev, irq, INTR_TYPE_TTY, NULL, siba_cc_intr, NULL,
	    &cookie);
	if (err != 0) {
		device_printf(dev, "unable to setup intr\n");
		return (ENXIO);
	}

	/* TODO: attach uart child */

	return (0);
}

static void
siba_cc_intr(void *v)
{

}

static device_method_t siba_cc_methods[] = {
	/* Device interface */
	DEVMETHOD(device_attach,	siba_cc_attach),
	DEVMETHOD(device_probe,		siba_cc_probe),

	DEVMETHOD_END
};

static driver_t siba_cc_driver = {
	"siba_cc",
	siba_cc_methods,
	sizeof(struct siba_softc),
};
static devclass_t siba_cc_devclass;

DRIVER_MODULE(siba_cc, siba, siba_cc_driver, siba_cc_devclass, 0, 0);