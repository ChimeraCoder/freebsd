
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
 * Glue allowing devices beneath the scalable shared memory node to be
 * treated like nexus(4) children
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <dev/ofw/ofw_bus.h>

#include <machine/nexusvar.h>

static device_probe_t ssm_probe;

static device_method_t ssm_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		ssm_probe),

	/* Bus interface */

	/* ofw_bus interface */

	DEVMETHOD_END
};

static devclass_t ssm_devclass;

DEFINE_CLASS_1(ssm, ssm_driver, ssm_methods, 1 /* no softc */, nexus_driver);
EARLY_DRIVER_MODULE(ssm, nexus, ssm_driver, ssm_devclass, 0, 0, BUS_PASS_BUS);
MODULE_DEPEND(ssm, nexus, 1, 1, 1);
MODULE_VERSION(ssm, 1);

static int
ssm_probe(device_t dev)
{

	if (strcmp(ofw_bus_get_name(dev), "ssm") == 0) {
		device_set_desc(dev, "Scalable Shared Memory");
		return (BUS_PROBE_DEFAULT);
	}
	return (ENXIO);
}