
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
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/sysctl.h>
#include <machine/fdt.h>

#include <dev/fdt/fdt_common.h>
#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>

#include <arm/mv/mvreg.h>
#include <arm/mv/mvvar.h>

static struct resource_spec mvts_res[] = {
	{ SYS_RES_MEMORY,	0,	RF_ACTIVE },
	{ -1, 0 }
};

struct mvts_softc {
	device_t		sc_dev;
	struct resource		*sc_res[sizeof(mvts_res)];
};

static int
ts_probe(device_t dev)
{
	uint32_t d, r;

	if (!ofw_bus_is_compatible(dev, "mrvl,ts"))
		return (ENXIO);
	soc_id(&d, &r);
	switch (d) {
	case MV_DEV_88F6282:
		break;
	default:
		device_printf(dev, "unsupported SoC (ID: 0x%08X)!\n", d);
		return (ENXIO);
	}
	device_set_desc(dev, "Marvell Thermal Sensor");

	return (0);
}

#define	MV_TEMP_VALID_BIT	(1 << 9)
#define	MV_TEMP_SENS_OFFS	10
#define	MV_TEMP_SENS_MASK	0x1ff
#define	MV_TEMP_SENS_READ_MAX	16
#define	TZ_ZEROC		2732
#define	MV_TEMP_CONVERT(x)	((((322 - x) * 100000) / 13625) + TZ_ZEROC)

/*
 * MSB                                 LSB
 * 0000 0000 0000 0000 0000 0000 0000 0000
 *                             ^- valid bit
 *                  |---------|
 *                         ^--- temperature (9 bits)
 */

static int
ts_sysctl_handler(SYSCTL_HANDLER_ARGS)
{
	struct mvts_softc *sc;
	device_t dev;
	uint32_t ret, ret0;
	u_int val;
	int i;

	dev = (device_t)arg1;
	sc = device_get_softc(dev);
	val = TZ_ZEROC;

	ret = bus_read_4(sc->sc_res[0], 0);
	if ((ret & MV_TEMP_VALID_BIT) == 0) {
		device_printf(dev, "temperature sensor is broken.\n");
		goto ts_sysctl_handle_int;
	}
	ret0 = 0;
	for (i = 0; i < MV_TEMP_SENS_READ_MAX; i++) {
		ret = bus_read_4(sc->sc_res[0], 0);
		ret = (ret >> MV_TEMP_SENS_OFFS) & MV_TEMP_SENS_MASK;

		/*
		 * Successive reads should returns the same value except
		 * for the LSB when the sensor is normal.
		 */
		if (((ret0 ^ ret) & 0x1fe) == 0)
			break;
		else
			ret0 = ret;
	}
	if (i == MV_TEMP_SENS_READ_MAX) {
		device_printf(dev, "temperature sensor is unstable.\n");
		goto ts_sysctl_handle_int;
	}
	val = (u_int)MV_TEMP_CONVERT(ret);

ts_sysctl_handle_int:
	return (sysctl_handle_int(oidp, &val, 0, req));
}

static int
ts_attach(device_t dev)
{
	struct mvts_softc *sc;
	struct sysctl_ctx_list *ctx;
	int error;

	sc = device_get_softc(dev);
	sc->sc_dev = dev;
	error = bus_alloc_resources(dev, mvts_res, sc->sc_res);
	if (error) {
		device_printf(dev, "could not allocate resources\n");
		return (ENXIO);
	}
	ctx = device_get_sysctl_ctx(dev);
	SYSCTL_ADD_PROC(ctx, SYSCTL_CHILDREN(device_get_sysctl_tree(dev)),
	    OID_AUTO, "temperature", CTLTYPE_INT | CTLFLAG_RD, dev,
	    0, ts_sysctl_handler, "IK", "Current Temperature");

	return (0);
}

static int
ts_detach(device_t dev)
{

	return (0);
}

static device_method_t ts_methods[] = {
	DEVMETHOD(device_probe,		ts_probe),
	DEVMETHOD(device_attach,	ts_attach),
	DEVMETHOD(device_detach,	ts_detach),
	{0, 0},
};

static driver_t ts_driver = {
	"mvts",
	ts_methods,
	sizeof(struct mvts_softc),
};

static devclass_t ts_devclass;
DRIVER_MODULE(mvts, simplebus, ts_driver, ts_devclass, 0, 0);
MODULE_VERSION(mvts, 1);