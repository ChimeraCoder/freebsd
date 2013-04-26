
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
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>
#include <sys/time.h>
#include <dev/joy/joyvar.h>

#include <isa/isavar.h>
#include "isa_if.h"

static int joy_isa_probe (device_t);

static struct isa_pnp_id joy_ids[] = {
    {0x0100630e, "CSC0001 PnP Joystick"},	/* CSC0001 */
    {0x0101630e, "CSC0101 PnP Joystick"},	/* CSC0101 */
    {0x01100002, "ALS0110 PnP Joystick"},	/* @P@1001 */
    {0x01200002, "ALS0120 PnP Joystick"},	/* @P@2001 */
    {0x01007316, "ESS0001 PnP Joystick"},	/* ESS0001 */
    {0x2fb0d041, "Generic PnP Joystick"},	/* PNPb02f */
    {0x2200a865, "YMH0022 PnP Joystick"},	/* YMH0022 */
    {0x82719304, NULL},    			/* ADS7182 */
    {0}
};

static int
joy_isa_probe(device_t dev)
{
    if (ISA_PNP_PROBE(device_get_parent(dev), dev, joy_ids) == ENXIO)
        return ENXIO;
    return (joy_probe(dev));
}

static device_method_t joy_methods[] = {
    DEVMETHOD(device_probe,	joy_isa_probe),
    DEVMETHOD(device_attach,	joy_attach),
    DEVMETHOD(device_detach,	joy_detach),
    { 0, 0 }
};

static driver_t joy_isa_driver = {
    "joy",
    joy_methods,
    sizeof (struct joy_softc)
};

DRIVER_MODULE(joy, isa, joy_isa_driver, joy_devclass, 0, 0);
DRIVER_MODULE(joy, acpi, joy_isa_driver, joy_devclass, 0, 0);