
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
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/malloc.h>   
#include <sys/module.h>
#include <sys/rman.h>
#include <sys/sysctl.h>

#include <machine/bus.h>

#include <dev/cfi/cfi_var.h>

#include <arm/xscale/ixp425/ixp425reg.h>
#include <arm/xscale/ixp425/ixp425var.h>

static int
cfi_ixp4xx_probe(device_t dev)
{
	struct cfi_softc *sc = device_get_softc(dev);
	/*
	 * NB: we assume the boot loader sets up EXP_TIMING_CS0_OFFSET
	 * according to the flash on the board.  If it does not then it
	 * can be done here.
	 */
	if (bootverbose) {
		struct ixp425_softc *sa =
		    device_get_softc(device_get_parent(dev));
		device_printf(dev, "EXP_TIMING_CS0_OFFSET 0x%x\n",
		    EXP_BUS_READ_4(sa, EXP_TIMING_CS0_OFFSET));
	}
	sc->sc_width = 2;		/* NB: don't probe interface width */
	return cfi_probe(dev);
}

static device_method_t cfi_ixp4xx_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		cfi_ixp4xx_probe),
	DEVMETHOD(device_attach,	cfi_attach),
	DEVMETHOD(device_detach,	cfi_detach),

	DEVMETHOD_END
};

static driver_t cfi_ixp4xx_driver = {
	cfi_driver_name,
	cfi_ixp4xx_methods,
	sizeof(struct cfi_softc),
};
DRIVER_MODULE(cfi, ixp, cfi_ixp4xx_driver, cfi_devclass, 0, 0);