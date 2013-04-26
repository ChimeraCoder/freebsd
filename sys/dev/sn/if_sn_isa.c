
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
#include <sys/kernel.h>
#include <sys/socket.h>

#include <sys/module.h>
#include <sys/bus.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <net/ethernet.h> 
#include <net/if.h>
#include <net/if_arp.h>

#include <isa/isavar.h>

#include <dev/sn/if_snvar.h>

static int		sn_isa_probe(device_t);
static int		sn_isa_attach(device_t);

static int
sn_isa_probe (device_t dev)
{
	if (isa_get_logicalid(dev))		/* skip PnP probes */
		return (ENXIO);
	if (sn_probe(dev) != 0)
		return (ENXIO);
	return (0);
}

static int
sn_isa_attach (device_t dev)
{
 	struct sn_softc *sc = device_get_softc(dev);
	int err;

	sc->dev = dev;
	err = sn_activate(dev);
	if (err) {
		sn_deactivate(dev);
		return (err);
	}
	err = sn_attach(dev);
	if (err)
		sn_deactivate(dev);
	return (err);
}

static device_method_t sn_isa_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		sn_isa_probe),
	DEVMETHOD(device_attach,	sn_isa_attach),
	DEVMETHOD(device_detach,	sn_detach),

	{ 0, 0 }
};

static driver_t sn_isa_driver = {
	"sn",
	sn_isa_methods,
	sizeof(struct sn_softc),
};

extern devclass_t sn_devclass;

DRIVER_MODULE(sn, isa, sn_isa_driver, sn_devclass, 0, 0);
MODULE_DEPEND(sn, isa, 1, 1, 1);
MODULE_DEPEND(sn, ether, 1, 1, 1);