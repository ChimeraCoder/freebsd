
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
#include <sys/socket.h>

#include <sys/module.h>
#include <sys/bus.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <sys/rman.h> 

#include <cam/scsi/scsi_low.h>

#include <dev/stg/tmc18c30reg.h>
#include <dev/stg/tmc18c30var.h>
#include <dev/stg/tmc18c30.h>

#define	STG_HOSTID	7

devclass_t stg_devclass;

int
stg_alloc_resource(device_t dev)
{
	struct stg_softc *	sc = device_get_softc(dev);
	u_long			maddr, msize;
	int			error;

	sc->port_res = bus_alloc_resource_any(dev, SYS_RES_IOPORT, 
					      &sc->port_rid, RF_ACTIVE);
	if (sc->port_res == NULL) {
		stg_release_resource(dev);
		return(ENOMEM);
	}

	sc->irq_res = bus_alloc_resource_any(dev, SYS_RES_IRQ, &sc->irq_rid,
					     RF_ACTIVE);
	if (sc->irq_res == NULL) {
		stg_release_resource(dev);
		return(ENOMEM);
	}
	error = bus_get_resource(dev, SYS_RES_MEMORY, 0, &maddr, &msize);
	if (error) {
		return(0);      /* XXX */
	}

	/* no need to allocate memory if not configured */
	if (maddr == 0 || msize == 0) {
		return(0);
	}

	sc->mem_res = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &sc->mem_rid,
					     RF_ACTIVE);
	if (sc->mem_res == NULL) {
		stg_release_resource(dev);
		return(ENOMEM);
	}

	return(0);
}

void
stg_release_resource(device_t dev)
{
	struct stg_softc	*sc = device_get_softc(dev);

	if (sc->stg_intrhand)
		bus_teardown_intr(dev, sc->irq_res, sc->stg_intrhand);
	if (sc->port_res)
		bus_release_resource(dev, SYS_RES_IOPORT,
				     sc->port_rid, sc->port_res);
	if (sc->irq_res)
		bus_release_resource(dev, SYS_RES_IRQ,
				     sc->irq_rid, sc->irq_res);
	if (sc->mem_res)
		bus_release_resource(dev, SYS_RES_MEMORY,
				     sc->mem_rid, sc->mem_res);
	return;
}

int
stg_probe(device_t dev)
{
	int rv;
	struct stg_softc *sc = device_get_softc(dev);

	rv = stgprobesubr(rman_get_bustag(sc->port_res),
			  rman_get_bushandle(sc->port_res),
			  device_get_flags(dev));

	return rv;
}

int
stg_attach(device_t dev)
{
	struct stg_softc *sc;
	struct scsi_low_softc *slp;
	u_int32_t flags = device_get_flags(dev);
	intrmask_t s;
	char	dvname[16];

	sc = device_get_softc(dev);

	strcpy(dvname,"stg");

	slp = &sc->sc_sclow;
	slp->sl_dev = dev;
	sc->sc_iot = rman_get_bustag(sc->port_res);
	sc->sc_ioh = rman_get_bushandle(sc->port_res);

	slp->sl_hostid = STG_HOSTID;
	slp->sl_cfgflags = flags;

	s = splcam();
	stgattachsubr(sc);
	splx(s);

	return(STGIOSZ);
}

int
stg_detach (device_t dev)
{
	struct stg_softc *sc = device_get_softc(dev);
	intrmask_t s;

	s = splcam();
	scsi_low_deactivate((struct scsi_low_softc *)sc);
	scsi_low_dettach(&sc->sc_sclow);
	splx(s);
	stg_release_resource(dev);
	return (0);
}

void
stg_intr (void *arg)
{
	stgintr(arg);
	return;
}