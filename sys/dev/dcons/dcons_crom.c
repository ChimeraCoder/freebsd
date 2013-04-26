
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

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/conf.h>
#include <sys/malloc.h>

#include <sys/bus.h>
#include <machine/bus.h>

#ifdef __DragonFly__
#include <bus/firewire/firewire.h>
#include <bus/firewire/firewirereg.h>
#include <bus/firewire/iec13213.h>
#include "dcons.h"
#include "dcons_os.h"
#else
#include <dev/firewire/firewire.h>
#include <dev/firewire/firewirereg.h>
#include <dev/firewire/iec13213.h>
#include <dev/dcons/dcons.h>
#include <dev/dcons/dcons_os.h>
#endif

#include <sys/cons.h>

#define EXPOSE_IDT_ADDR 1

#if (defined(__i386__) || defined(__amd64__)) && defined(EXPOSE_IDT_ADDR)
#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>
#include <machine/segments.h> /* for idt */
#endif
static bus_addr_t dcons_paddr;

#if __FreeBSD_version >= 500000
static int force_console = 0;
TUNABLE_INT("hw.firewire.dcons_crom.force_console", &force_console);
#endif

#ifndef CSRVAL_VENDOR_PRIVATE
#define NEED_NEW_DRIVER
#endif

#define ADDR_HI(x)	(((x) >> 24) & 0xffffff)
#define ADDR_LO(x)	((x) & 0xffffff)

struct dcons_crom_softc {
        struct firewire_dev_comm fd;
	struct crom_chunk unit;
	struct crom_chunk spec;
	struct crom_chunk ver;
	bus_dma_tag_t dma_tag;
	bus_dmamap_t dma_map;
	bus_addr_t bus_addr;
	eventhandler_tag ehand;
};

static void
dcons_crom_identify(driver_t *driver, device_t parent)
{
	BUS_ADD_CHILD(parent, 0, "dcons_crom", device_get_unit(parent));
}

static int
dcons_crom_probe(device_t dev)
{
	device_t pa;

	pa = device_get_parent(dev);
	if(device_get_unit(dev) != device_get_unit(pa)){
		return(ENXIO);
	}

	device_set_desc(dev, "dcons configuration ROM");
	return (0);
}

#ifndef NEED_NEW_DRIVER
#if (defined(__i386__) || defined(__amd64__)) && defined(EXPOSE_IDT_ADDR)
static void
dcons_crom_expose_idt(struct dcons_crom_softc *sc)
{
	static off_t idt_paddr;

	/* XXX */
	idt_paddr = (char *)idt - (char *)KERNBASE;

	crom_add_entry(&sc->unit, DCONS_CSR_KEY_RESET_HI, ADDR_HI(idt_paddr));
	crom_add_entry(&sc->unit, DCONS_CSR_KEY_RESET_LO, ADDR_LO(idt_paddr));
}
#endif
static void
dcons_crom_post_busreset(void *arg)
{
	struct dcons_crom_softc *sc;
	struct crom_src *src;
	struct crom_chunk *root;

	sc = (struct dcons_crom_softc *) arg;
	src = sc->fd.fc->crom_src;
	root = sc->fd.fc->crom_root;

	bzero(&sc->unit, sizeof(struct crom_chunk));

	crom_add_chunk(src, root, &sc->unit, CROM_UDIR);
	crom_add_entry(&sc->unit, CSRKEY_SPEC, CSRVAL_VENDOR_PRIVATE);
	crom_add_simple_text(src, &sc->unit, &sc->spec, "FreeBSD");
	crom_add_entry(&sc->unit, CSRKEY_VER, DCONS_CSR_VAL_VER);
	crom_add_simple_text(src, &sc->unit, &sc->ver, "dcons");
	crom_add_entry(&sc->unit, DCONS_CSR_KEY_HI, ADDR_HI(dcons_paddr));
	crom_add_entry(&sc->unit, DCONS_CSR_KEY_LO, ADDR_LO(dcons_paddr));
#if (defined(__i386__) || defined(__amd64__)) && defined(EXPOSE_IDT_ADDR)
	dcons_crom_expose_idt(sc);
#endif
}
#endif

static void
dmamap_cb(void *arg, bus_dma_segment_t *segments, int seg, int error)
{
	struct dcons_crom_softc *sc;

	if (error)
		printf("dcons_dmamap_cb: error=%d\n", error);

	sc = (struct dcons_crom_softc *)arg;
	sc->bus_addr = segments[0].ds_addr;

	bus_dmamap_sync(sc->dma_tag, sc->dma_map, BUS_DMASYNC_PREWRITE);
	device_printf(sc->fd.dev,
#if __FreeBSD_version < 500000
	    "bus_addr 0x%x\n", sc->bus_addr);
#else
	    "bus_addr 0x%jx\n", (uintmax_t)sc->bus_addr);
#endif
	if (dcons_paddr != 0) {
		/* XXX */
		device_printf(sc->fd.dev, "dcons_paddr is already set\n");
		return;
	}
	dcons_conf->dma_tag = sc->dma_tag;
	dcons_conf->dma_map = sc->dma_map;
	dcons_paddr = sc->bus_addr;

#if __FreeBSD_version >= 500000
	/* Force to be the high-level console */
	if (force_console)
		cnselect(dcons_conf->cdev);
#endif
}

static void
dcons_crom_poll(void *p, int arg)
{
	struct dcons_crom_softc *sc = (struct dcons_crom_softc *) p;

	sc->fd.fc->poll(sc->fd.fc, -1, -1);
}

static int
dcons_crom_attach(device_t dev)
{
#ifdef NEED_NEW_DRIVER
	printf("dcons_crom: you need newer firewire driver\n");
	return (-1);
#else
	struct dcons_crom_softc *sc;
	int error;

	if (dcons_conf->buf == NULL)
		return (ENXIO);
        sc = (struct dcons_crom_softc *) device_get_softc(dev);
	sc->fd.fc = device_get_ivars(dev);
	sc->fd.dev = dev;
	sc->fd.post_explore = NULL;
	sc->fd.post_busreset = (void *) dcons_crom_post_busreset;

	/* map dcons buffer */
	error = bus_dma_tag_create(
		/*parent*/ sc->fd.fc->dmat,
		/*alignment*/ sizeof(u_int32_t),
		/*boundary*/ 0,
		/*lowaddr*/ BUS_SPACE_MAXADDR,
		/*highaddr*/ BUS_SPACE_MAXADDR,
		/*filter*/NULL, /*filterarg*/NULL,
		/*maxsize*/ dcons_conf->size,
		/*nsegments*/ 1,
		/*maxsegsz*/ BUS_SPACE_MAXSIZE_32BIT,
		/*flags*/ BUS_DMA_ALLOCNOW,
#if __FreeBSD_version >= 501102
		/*lockfunc*/busdma_lock_mutex,
		/*lockarg*/&Giant,
#endif
		&sc->dma_tag);
	if (error != 0)
		return (error);
	error = bus_dmamap_create(sc->dma_tag, BUS_DMA_COHERENT, &sc->dma_map);
	if (error != 0)
		return (error);
	error = bus_dmamap_load(sc->dma_tag, sc->dma_map,
	    (void *)dcons_conf->buf, dcons_conf->size,
	    dmamap_cb, sc, 0);
	if (error != 0)
		return (error);
	sc->ehand = EVENTHANDLER_REGISTER(dcons_poll, dcons_crom_poll,
			 (void *)sc, 0);
	return (0);
#endif
}

static int
dcons_crom_detach(device_t dev)
{
	struct dcons_crom_softc *sc;

        sc = (struct dcons_crom_softc *) device_get_softc(dev);
	sc->fd.post_busreset = NULL;

	if (sc->ehand)
		EVENTHANDLER_DEREGISTER(dcons_poll, sc->ehand);

	/* XXX */
	if (dcons_conf->dma_tag == sc->dma_tag)
		dcons_conf->dma_tag = NULL;

	bus_dmamap_unload(sc->dma_tag, sc->dma_map);
	bus_dmamap_destroy(sc->dma_tag, sc->dma_map);
	bus_dma_tag_destroy(sc->dma_tag);

	return 0;
}

static devclass_t dcons_crom_devclass;

static device_method_t dcons_crom_methods[] = {
	/* device interface */
	DEVMETHOD(device_identify,	dcons_crom_identify),
	DEVMETHOD(device_probe,		dcons_crom_probe),
	DEVMETHOD(device_attach,	dcons_crom_attach),
	DEVMETHOD(device_detach,	dcons_crom_detach),
	{ 0, 0 }
};

static driver_t dcons_crom_driver = {
	"dcons_crom",
	dcons_crom_methods,
	sizeof(struct dcons_crom_softc),
};

DRIVER_MODULE(dcons_crom, firewire, dcons_crom_driver,
					dcons_crom_devclass, 0, 0);
MODULE_VERSION(dcons_crom, 1);
MODULE_DEPEND(dcons_crom, dcons,
	DCONS_VERSION, DCONS_VERSION, DCONS_VERSION);
MODULE_DEPEND(dcons_crom, firewire, 1, 1, 1);