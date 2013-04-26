
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
#include <sys/lock.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/bus.h>
#include <machine/bus.h>
#include <sys/malloc.h>
#include <sys/rman.h>

#include <isa/isavar.h>

#include <dev/ppbus/ppbconf.h>
#include <dev/ppbus/ppb_msq.h>
#include <dev/ppc/ppcvar.h>
#include <dev/ppc/ppcreg.h>

#include "ppbus_if.h"

static int ppc_isa_probe(device_t dev);

int ppc_isa_attach(device_t dev);
int ppc_isa_write(device_t, char *, int, int);

static device_method_t ppc_isa_methods[] = {
	/* device interface */
	DEVMETHOD(device_probe,		ppc_isa_probe),
	DEVMETHOD(device_attach,	ppc_isa_attach),
	DEVMETHOD(device_detach,	ppc_detach),

	/* bus interface */
	DEVMETHOD(bus_read_ivar,	ppc_read_ivar),
	DEVMETHOD(bus_write_ivar,	ppc_write_ivar),
	DEVMETHOD(bus_alloc_resource,	ppc_alloc_resource),
	DEVMETHOD(bus_release_resource,	ppc_release_resource),

	/* ppbus interface */
	DEVMETHOD(ppbus_io,		ppc_io),
	DEVMETHOD(ppbus_exec_microseq,	ppc_exec_microseq),
	DEVMETHOD(ppbus_reset_epp,	ppc_reset_epp),
	DEVMETHOD(ppbus_setmode,	ppc_setmode),
	DEVMETHOD(ppbus_ecp_sync,	ppc_ecp_sync),
	DEVMETHOD(ppbus_read,		ppc_read),
	DEVMETHOD(ppbus_write,		ppc_isa_write),

	{ 0, 0 }
};

static driver_t ppc_isa_driver = {
	ppc_driver_name,
	ppc_isa_methods,
	sizeof(struct ppc_data),
};

static struct isa_pnp_id lpc_ids[] = {
	{ 0x0004d041, "Standard parallel printer port" },	/* PNP0400 */
	{ 0x0104d041, "ECP parallel printer port" },		/* PNP0401 */
	{ 0 }
};

static void
ppc_isa_dmadone(struct ppc_data *ppc)
{
	isa_dmadone(ppc->ppc_dmaflags, ppc->ppc_dmaddr, ppc->ppc_dmacnt,
	    ppc->ppc_dmachan);
}

int
ppc_isa_attach(device_t dev)
{
	struct ppc_data *ppc = device_get_softc(dev);

	if ((ppc->ppc_avm & PPB_ECP) && (ppc->ppc_dmachan > 0)) {
		/* acquire the DMA channel forever */   /* XXX */
		isa_dma_acquire(ppc->ppc_dmachan);
		isa_dmainit(ppc->ppc_dmachan, 1024); /* nlpt.BUFSIZE */
		ppc->ppc_dmadone = ppc_isa_dmadone;
	}

	return (ppc_attach(dev));
}

static int
ppc_isa_probe(device_t dev)
{
	device_t parent;
	int error;

	parent = device_get_parent(dev);

	error = ISA_PNP_PROBE(parent, dev, lpc_ids);
	if (error == ENXIO)
		return (ENXIO);
	if (error != 0)		/* XXX shall be set after detection */
		device_set_desc(dev, "Parallel port");

	return (ppc_probe(dev, 0));
}

/*
 * Call this function if you want to send data in any advanced mode
 * of your parallel port: FIFO, DMA
 *
 * If what you want is not possible (no ECP, no DMA...),
 * EINVAL is returned
 */
int
ppc_isa_write(device_t dev, char *buf, int len, int how)
{
	struct ppc_data *ppc = device_get_softc(dev);
	char ecr, ecr_sav, ctr, ctr_sav;
	int error = 0;
	int spin;

	PPC_ASSERT_LOCKED(ppc);
	if (!(ppc->ppc_avm & PPB_ECP))
		return (EINVAL);
	if (ppc->ppc_dmachan == 0)
		return (EINVAL);

#ifdef PPC_DEBUG
	printf("w");
#endif

	ecr_sav = r_ecr(ppc);
	ctr_sav = r_ctr(ppc);

	/*
	 * Send buffer with DMA, FIFO and interrupts
	 */

	/* byte mode, no intr, no DMA, dir=0, flush fifo */
	ecr = PPC_ECR_STD | PPC_DISABLE_INTR;
	w_ecr(ppc, ecr);

	/* disable nAck interrupts */
	ctr = r_ctr(ppc);
	ctr &= ~IRQENABLE;
	w_ctr(ppc, ctr);

	ppc->ppc_dmaflags = 0;
	ppc->ppc_dmaddr = (caddr_t)buf;
	ppc->ppc_dmacnt = (u_int)len;

	switch (ppc->ppc_mode) {
	case PPB_COMPATIBLE:
		/* compatible mode with FIFO, no intr, DMA, dir=0 */
		ecr = PPC_ECR_FIFO | PPC_DISABLE_INTR | PPC_ENABLE_DMA;
		break;
	case PPB_ECP:
		ecr = PPC_ECR_ECP | PPC_DISABLE_INTR | PPC_ENABLE_DMA;
		break;
	default:
		error = EINVAL;
		goto error;
	}

	w_ecr(ppc, ecr);
	ecr = r_ecr(ppc);

	ppc->ppc_dmastat = PPC_DMA_INIT;

	/* enable interrupts */
	ecr &= ~PPC_SERVICE_INTR;
	ppc->ppc_irqstat = PPC_IRQ_DMA;
	w_ecr(ppc, ecr);

	isa_dmastart(ppc->ppc_dmaflags, ppc->ppc_dmaddr, ppc->ppc_dmacnt,
		     ppc->ppc_dmachan);
	ppc->ppc_dmastat = PPC_DMA_STARTED;

#ifdef PPC_DEBUG
	printf("s%d", ppc->ppc_dmacnt);
#endif

	/* Wait for the DMA completed interrupt. We hope we won't
	 * miss it, otherwise a signal will be necessary to unlock the
	 * process.
	 */
	do {
		/* release CPU */
		error = mtx_sleep(ppc, &ppc->ppc_lock, PPBPRI | PCATCH,
		    "ppcdma", 0);
	} while (error == EWOULDBLOCK);

	if (error) {
#ifdef PPC_DEBUG
		printf("i");
#endif
		/* stop DMA */
		isa_dmadone(ppc->ppc_dmaflags, ppc->ppc_dmaddr,
			    ppc->ppc_dmacnt, ppc->ppc_dmachan);

		/* no dma, no interrupt, flush the fifo */
		w_ecr(ppc, PPC_ECR_RESET);

		ppc->ppc_dmastat = PPC_DMA_INTERRUPTED;
		goto error;
	}

	/* wait for an empty fifo */
	while (!(r_ecr(ppc) & PPC_FIFO_EMPTY)) {

		for (spin=100; spin; spin--)
			if (r_ecr(ppc) & PPC_FIFO_EMPTY)
				goto fifo_empty;
#ifdef PPC_DEBUG
		printf("Z");
#endif
		error = mtx_sleep(ppc, &ppc->ppc_lock, PPBPRI | PCATCH,
		    "ppcfifo", hz / 100);
		if (error != EWOULDBLOCK) {
#ifdef PPC_DEBUG
			printf("I");
#endif
			/* no dma, no interrupt, flush the fifo */
			w_ecr(ppc, PPC_ECR_RESET);

			ppc->ppc_dmastat = PPC_DMA_INTERRUPTED;
			error = EINTR;
			goto error;
		}
	}

fifo_empty:
	/* no dma, no interrupt, flush the fifo */
	w_ecr(ppc, PPC_ECR_RESET);

error:
	/* PDRQ must be kept unasserted until nPDACK is
	 * deasserted for a minimum of 350ns (SMC datasheet)
	 *
	 * Consequence may be a FIFO that never empty
	 */
	DELAY(1);

	w_ecr(ppc, ecr_sav);
	w_ctr(ppc, ctr_sav);
	return (error);
}

DRIVER_MODULE(ppc, isa, ppc_isa_driver, ppc_devclass, 0, 0);