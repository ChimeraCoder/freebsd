
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
#include <sys/cons.h>
#include <sys/consio.h>
#include <sys/kernel.h>
#include <sys/limits.h>
#include <sys/module.h>

#include <dev/ofw/ofw_bus.h>

#include <machine/bus.h>

#include <dev/syscons/syscons.h>

#define	SC_MD_MAX	8
#define	SC_MD_FLAGS	SC_AUTODETECT_KBD

static sc_softc_t sc_softcs[SC_MD_MAX];

static device_identify_t sc_identify;
static device_probe_t sc_probe;
static device_attach_t sc_attach;

static device_method_t sc_methods[] = {
	/* Device interface */
	DEVMETHOD(device_identify,	sc_identify),
	DEVMETHOD(device_probe,		sc_probe),
	DEVMETHOD(device_attach,	sc_attach),

	DEVMETHOD_END
};

static driver_t sc_driver = {
	SC_DRIVER_NAME,
	sc_methods,
	1,	/* no softc */
};

static devclass_t sc_devclass;

DRIVER_MODULE(sc, nexus, sc_driver, sc_devclass, 0, 0);

static void
sc_identify(driver_t *driver, device_t parent)
{

	/*
	 * Add with a priority guaranteed to make it last on
	 * the device list.
	 */
	BUS_ADD_CHILD(parent, INT_MAX, SC_DRIVER_NAME, 0);
}

static int
sc_probe(device_t dev)
{
	int unit;

	unit = device_get_unit(dev);
	if (strcmp(ofw_bus_get_name(dev), SC_DRIVER_NAME) != 0 ||
	    unit >= SC_MD_MAX)
		return (ENXIO);

	device_set_desc(dev, "System console");
	return (sc_probe_unit(unit, device_get_flags(dev) | SC_MD_FLAGS));
}

static int
sc_attach(device_t dev)
{

	return (sc_attach_unit(device_get_unit(dev),
	    device_get_flags(dev) | SC_MD_FLAGS));
}

int
sc_get_cons_priority(int *unit, int *flags)
{

	*unit = 0;
	*flags = 0;
	return (CN_INTERNAL);
}

int
sc_max_unit(void)
{

	return (devclass_get_maxunit(sc_devclass));
}

sc_softc_t *
sc_get_softc(int unit, int flags)
{
	sc_softc_t *sc;

	if (unit < 0 || unit >= SC_MD_MAX)
		return (NULL);
	sc = &sc_softcs[unit];
	sc->unit = unit;
	if ((sc->flags & SC_INIT_DONE) == 0) {
		sc->keyboard = -1;
		sc->adapter = -1;
		sc->cursor_char = SC_CURSOR_CHAR;
		sc->mouse_char = SC_MOUSE_CHAR;
	}
	return (sc);
}

void
sc_get_bios_values(bios_values_t *values)
{

}

int
sc_tone(int hz)
{

	return (0);
}