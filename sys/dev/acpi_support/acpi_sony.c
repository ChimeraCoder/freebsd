
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

#include "opt_acpi.h"
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/bus.h>

#include <contrib/dev/acpica/include/acpi.h>

#include "acpi_if.h"
#include <sys/module.h>
#include <dev/acpica/acpivar.h>
#include <sys/sysctl.h>

#define _COMPONENT	ACPI_OEM
ACPI_MODULE_NAME("Sony")

#define ACPI_SONY_GET_PID "GPID"

/*
 * SNY5001
 *   This is the ACPI handle for the "Sony Notebook Control" driver under
 *   Windows.
 *   It provides several methods within the ACPI namespace, including:
 *  [GS]BRT [GS]PBR [GS]CTR [GS]PCR [GS]CMI [CDPW GCDP]? GWDP PWAK PWRN 
 *
 * SNY6001
 *   This is the ACPI handle for the "Sony Programmable I/O" driver under
 *   Windows.
 *   It is not yet supported by this driver, but provides control over the
 *   power to the bluetooth, built-in camera and HSDPA modem devices in some
 *   laptops, and also allows some control of the fan speed.
 */

struct acpi_sony_softc {
	int pid;
};
static struct acpi_sony_name_list
{
	char *nodename;
	char *getmethod;
	char *setmethod;
	char *comment;
} acpi_sony_oids[] = {
	{ "brightness", "GBRT", "SBRT", "Display Brightness"},
	{ "brightness_default", "GPBR", "SPBR", "Default Display Brightness"},
	{ "contrast", "GCTR", "SCTR", "Display Contrast"},
	{ "bass_gain", "GMGB", "SMGB", "Multimedia Bass Gain"},
	{ "pcr", "GPCR", "SPCR", "???"},
#if 0
	{ "cmi", "GCMI", "SCMI", "???"},
#endif
	{ "wdp", "GWDP", NULL, "???"},
	{ "cdp", "GCDP", "CDPW", "CD Power"},  /*shares [\GL03]&0x8 flag*/
	{ "azp", "GAZP", "AZPW", "Audio Power"}, 
	{ "lnp", "GLNP", "LNPW", "LAN Power"},
	{ NULL, NULL, NULL }
};

static int	acpi_sony_probe(device_t dev);
static int	acpi_sony_attach(device_t dev);
static int 	acpi_sony_detach(device_t dev);
static int	sysctl_acpi_sony_gen_handler(SYSCTL_HANDLER_ARGS);

static device_method_t acpi_sony_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe, acpi_sony_probe),
	DEVMETHOD(device_attach, acpi_sony_attach),
	DEVMETHOD(device_detach, acpi_sony_detach),

	DEVMETHOD_END
};

static driver_t	acpi_sony_driver = {
	"acpi_sony",
	acpi_sony_methods,
	sizeof(struct acpi_sony_softc),
};

static devclass_t acpi_sony_devclass;

DRIVER_MODULE(acpi_sony, acpi, acpi_sony_driver, acpi_sony_devclass,
	      0, 0);
MODULE_DEPEND(acpi_sony, acpi, 1, 1, 1);
static char    *sny_id[] = {"SNY5001", NULL};

static int
acpi_sony_probe(device_t dev)
{
	int ret = ENXIO;

	if (ACPI_ID_PROBE(device_get_parent(dev), dev, sny_id)) {
		device_set_desc(dev, "Sony notebook controller");
		ret = 0;
	}
	return (ret);
}

static int
acpi_sony_attach(device_t dev)
{
	struct acpi_sony_softc *sc;
	int i;

	sc = device_get_softc(dev);
	acpi_GetInteger(acpi_get_handle(dev), ACPI_SONY_GET_PID, &sc->pid);
	device_printf(dev, "PID %x\n", sc->pid);
	for (i = 0 ; acpi_sony_oids[i].nodename != NULL; i++){
		SYSCTL_ADD_PROC(device_get_sysctl_ctx(dev),
		    SYSCTL_CHILDREN(device_get_sysctl_tree(dev)),
		    i, acpi_sony_oids[i].nodename , CTLTYPE_INT |
		    ((acpi_sony_oids[i].setmethod)? CTLFLAG_RW: CTLFLAG_RD),
		    dev, i, sysctl_acpi_sony_gen_handler, "I",
		    acpi_sony_oids[i].comment);
	}
	return (0);
}

static int 
acpi_sony_detach(device_t dev)
{
	return (0);
}

#if 0
static int
acpi_sony_suspend(device_t dev)
{
	struct acpi_sony_softc *sc = device_get_softc(dev);
	return (0);
}

static int
acpi_sony_resume(device_t dev)
{
	return (0);
}
#endif

static int 
sysctl_acpi_sony_gen_handler(SYSCTL_HANDLER_ARGS)
{
	device_t	dev = arg1;
	int 	function = oidp->oid_arg2;
	int		error = 0, val;

	acpi_GetInteger(acpi_get_handle(dev),
	    acpi_sony_oids[function].getmethod, &val);
	error = sysctl_handle_int(oidp, &val, 0, req);
	if (error || !req->newptr || !acpi_sony_oids[function].setmethod)
		return (error);
	acpi_SetInteger(acpi_get_handle(dev),
	    acpi_sony_oids[function].setmethod, val);
	return (0);
}