
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
#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h>
#include <dev/joy/joyvar.h>

#include <dev/pccard/pccardvar.h>

#include "pccarddevs.h"

static int
joy_pccard_probe(device_t dev)
{
	/* For the moment, don't match anything :-) */
	return(ENXIO);
}

static device_method_t joy_pccard_methods[] = {
	DEVMETHOD(device_probe,		joy_pccard_probe),
	DEVMETHOD(device_attach,	joy_attach),
	DEVMETHOD(device_detach,	joy_detach),
	{ 0, 0 }
};

static driver_t joy_pccard_driver = {
    "joy",
    joy_pccard_methods,
    sizeof (struct joy_softc)
};

DRIVER_MODULE(joy, pccard, joy_pccard_driver, joy_devclass, 0, 0);