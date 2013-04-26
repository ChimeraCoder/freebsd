
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
#include <sys/malloc.h>
#include <sys/proc.h>

#include <sys/module.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/bus.h>

#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>

#include <machine/md_var.h>

#include <isa/isareg.h>
#include <isa/isavar.h>
#include "tpmvar.h"

#include "opt_acpi.h"
#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>
#include <dev/acpica/acpivar.h>



char *tpm_ids[] = {"ATM1200",  "BCM0102", "INTC0102", "SNO3504", "WEC1000",
    "PNP0C31", NULL};

static int
tpm_acpi_probe(device_t dev)
{
	if (ACPI_ID_PROBE(device_get_parent(dev), dev, tpm_ids) != NULL) {
		device_set_desc(dev, "Trusted Platform Module");
		return BUS_PROBE_DEFAULT;
	}
	
	return ENXIO;
}

static device_method_t tpm_acpi_methods[] = {
#if 0
	/*In some case, TPM existance is found only in TPCA header*/
	DEVMETHOD(device_identify,	tpm_acpi_identify),
#endif

	DEVMETHOD(device_probe,		tpm_acpi_probe),
	DEVMETHOD(device_attach,	tpm_attach),
	DEVMETHOD(device_detach,	tpm_detach),
	DEVMETHOD(device_suspend,	tpm_suspend),
	DEVMETHOD(device_resume,	tpm_resume),
	{ 0, 0 }
};
static driver_t tpm_acpi_driver = {
	"tpm", tpm_acpi_methods, sizeof(struct tpm_softc),
};

devclass_t tpm_devclass;
DRIVER_MODULE(tpm, acpi, tpm_acpi_driver, tpm_devclass, 0, 0);