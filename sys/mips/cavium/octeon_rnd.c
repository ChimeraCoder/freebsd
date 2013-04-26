
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
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/random.h>

#include <contrib/octeon-sdk/cvmx.h>
#include <contrib/octeon-sdk/cvmx-rng.h>

/*
 * XXX
 * random_harvest(9) says to call it with no more than 16 bytes, but at least
 * safe(4) seems to violate that rule.
 */
#define	OCTEON_RND_WORDS	2

struct octeon_rnd_softc {
	uint64_t sc_entropy[OCTEON_RND_WORDS];
	struct callout sc_callout;
};

static void	octeon_rnd_identify(driver_t *drv, device_t parent);
static int	octeon_rnd_attach(device_t dev);
static int	octeon_rnd_probe(device_t dev);
static int	octeon_rnd_detach(device_t dev);

static void	octeon_rnd_harvest(void *);

static device_method_t octeon_rnd_methods[] = {
	/* Device interface */
	DEVMETHOD(device_identify,	octeon_rnd_identify),
	DEVMETHOD(device_probe,		octeon_rnd_probe),
	DEVMETHOD(device_attach,	octeon_rnd_attach),
	DEVMETHOD(device_detach,	octeon_rnd_detach),

	{ 0, 0 }
};

static driver_t octeon_rnd_driver = {
	"rnd",
	octeon_rnd_methods,
	sizeof (struct octeon_rnd_softc)
};
static devclass_t octeon_rnd_devclass;
DRIVER_MODULE(rnd, nexus, octeon_rnd_driver, octeon_rnd_devclass, 0, 0);

static void
octeon_rnd_identify(driver_t *drv, device_t parent)
{
	BUS_ADD_CHILD(parent, 0, "rnd", 0);
}

static int
octeon_rnd_probe(device_t dev)
{
	if (device_get_unit(dev) != 0)
		return (ENXIO);

	device_set_desc(dev, "Cavium Octeon Random Number Generator");
	return (0);
}

static int
octeon_rnd_attach(device_t dev)
{
	struct octeon_rnd_softc *sc;

	sc = device_get_softc(dev);
	callout_init(&sc->sc_callout, CALLOUT_MPSAFE);
	callout_reset(&sc->sc_callout, hz * 5, octeon_rnd_harvest, sc);

	cvmx_rng_enable();

	return (0);
}

static int
octeon_rnd_detach(device_t dev)
{
	struct octeon_rnd_softc *sc;

	sc = device_get_softc(dev);

	callout_stop(&sc->sc_callout);

	return (0);
}

static void
octeon_rnd_harvest(void *arg)
{
	struct octeon_rnd_softc *sc;
	unsigned i;

	sc = arg;

	for (i = 0; i < OCTEON_RND_WORDS; i++)
		sc->sc_entropy[i] = cvmx_rng_get_random64();
	random_harvest(sc->sc_entropy, sizeof sc->sc_entropy,
		       sizeof sc->sc_entropy * 8, 0, RANDOM_PURE);

	callout_reset(&sc->sc_callout, hz * 5, octeon_rnd_harvest, sc);
}