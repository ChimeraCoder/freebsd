
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

#include <dev/cs/if_csvar.h>
#include <dev/cs/if_csreg.h>

static int		cs_isa_probe(device_t);
static int		cs_isa_attach(device_t);

static struct isa_pnp_id cs_ids[] = {
	{ 0x4060630e, NULL },		/* CSC6040 */
	{ 0x10104d24, NULL },		/* IBM EtherJet */
	{ 0, NULL }
};

/*
 * Determine if the device is present
 */
static int
cs_isa_probe(device_t dev)
{
	int error;

	/* Check isapnp ids */
	error = ISA_PNP_PROBE(device_get_parent(dev), dev, cs_ids);

	/* If the card had a PnP ID that didn't match any we know about */
	if (error == ENXIO)
                goto end;

        /* If we've matched, or there's no PNP ID, probe chip */
	if (error == 0 || error == ENOENT)
		error = cs_cs89x0_probe(dev);
end:
	/* Make sure IRQ is assigned for probe message and available */
	if (error == 0)
                error = cs_alloc_irq(dev, 0);

        cs_release_resources(dev);
        return (error);
}

static int
cs_isa_attach(device_t dev)
{
        struct cs_softc *sc = device_get_softc(dev);
        
	cs_alloc_port(dev, 0, CS_89x0_IO_PORTS);
        cs_alloc_irq(dev, sc->irq_rid);
                
        return (cs_attach(dev));
}

static device_method_t cs_isa_methods[] = {
	/* Device interface */
	DEVMETHOD(device_probe,		cs_isa_probe),
	DEVMETHOD(device_attach,	cs_isa_attach),
	DEVMETHOD(device_detach,	cs_detach),

	{ 0, 0 }
};

static driver_t cs_isa_driver = {
	"cs",
	cs_isa_methods,
	sizeof(struct cs_softc),
};

extern devclass_t cs_devclass;

DRIVER_MODULE(cs, isa, cs_isa_driver, cs_devclass, 0, 0);
MODULE_DEPEND(cs, isa, 1, 1, 1);
MODULE_DEPEND(cs, ether, 1, 1, 1);