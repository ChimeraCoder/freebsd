
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

/*
 *	Apple Onboard Audio (AOA).
 */

#include <sys/cdefs.h>

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/bus.h>
#include <sys/malloc.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <machine/dbdma.h>
#include <machine/resource.h>
#include <machine/bus.h>
#include <sys/rman.h>
#include <dev/ofw/ofw_bus.h>

#ifdef HAVE_KERNEL_OPTION_HEADERS
#include "opt_snd.h"
#endif

#include <dev/sound/pcm/sound.h>
#include <dev/sound/macio/aoa.h>

#include "mixer_if.h"

struct aoa_dma {
	struct mtx 		 mutex;
	struct resource 	*reg; 		/* DBDMA registers */
	dbdma_channel_t 	*channel; 	/* DBDMA channel */
	bus_dma_tag_t 		 tag; 		/* bus_dma tag */
	struct pcm_channel 	*pcm;		/* PCM channel */
	struct snd_dbuf		*buf; 		/* PCM buffer */
	u_int 			 slots; 	/* # of slots */
	u_int 			 slot;		/* current slot */
	u_int 			 bufsz; 	/* buffer size */
	u_int 			 blksz; 	/* block size */
	int 			 running;
};

static void
aoa_dma_set_program(struct aoa_dma *dma)
{
	u_int32_t 		 addr;
	int 			 i;

	addr = (u_int32_t) sndbuf_getbufaddr(dma->buf);
	KASSERT(dma->bufsz == sndbuf_getsize(dma->buf), ("bad size"));

	dma->slots = dma->bufsz / dma->blksz;

	for (i = 0; i < dma->slots; ++i) {
		dbdma_insert_command(dma->channel, 
		    i, /* slot */
		    DBDMA_OUTPUT_MORE, /* command */
		    0, /* stream */
		    addr, /* data */
		    dma->blksz, /* count */
		    DBDMA_ALWAYS, /* interrupt */
		    DBDMA_COND_TRUE, /* branch */
		    DBDMA_NEVER, /* wait */
		    dma->slots + 1 /* branch_slot */
		);

		addr += dma->blksz;
	}

	/* Branch back to beginning. */
	dbdma_insert_branch(dma->channel, dma->slots, 0);

	/* STOP command to branch when S0 is asserted. */
	dbdma_insert_stop(dma->channel, dma->slots + 1);

	/* Set S0 as the condition to branch to STOP. */
	dbdma_set_branch_selector(dma->channel, 1 << 0, 1 << 0);
	dbdma_set_device_status(dma->channel, 1 << 0, 0);

	dbdma_sync_commands(dma->channel, BUS_DMASYNC_PREWRITE);
}

#define AOA_BUFFER_SIZE		65536

static struct aoa_dma * 
aoa_dma_create(struct aoa_softc *sc)
{
	struct aoa_dma *dma;
	bus_dma_tag_t 	tag;
	int 		err;
	device_t	self;

	self = sc->sc_dev;
	err = bus_dma_tag_create(bus_get_dma_tag(self), 
	    4, 0, BUS_SPACE_MAXADDR_32BIT, BUS_SPACE_MAXADDR, NULL, NULL, 
	    AOA_BUFFER_SIZE, 1, AOA_BUFFER_SIZE, 0, NULL, NULL, &tag);
	if (err != 0) 
		return (NULL);

	dma = malloc(sizeof(*dma), M_DEVBUF, M_WAITOK | M_ZERO);
	dma->tag = tag;
	dma->bufsz = AOA_BUFFER_SIZE;
	dma->blksz = PAGE_SIZE; /* initial blocksize */
	
	mtx_init(&dma->mutex, "AOA", NULL, MTX_DEF);

	sc->sc_intrp = dma;

	return (dma);
}

static void
aoa_dma_delete(struct aoa_dma *dma)
{
	bus_dma_tag_destroy(dma->tag);
	mtx_destroy(&dma->mutex);
	free(dma, M_DEVBUF);
}

static u_int32_t
aoa_chan_setblocksize(kobj_t obj, void *data, u_int32_t blocksz)
{
	struct aoa_dma 		*dma = data;
	int 			 err, lz;

	DPRINTF(("aoa_chan_setblocksize: blocksz = %u, dma->blksz = %u\n", 
		blocksz, dma->blksz));
	KASSERT(!dma->running, ("dma is running"));
	KASSERT(blocksz > 0, ("bad blocksz"));

	/* Round blocksz down to a power of two... */
	__asm volatile ("cntlzw %0,%1" : "=r"(lz) : "r"(blocksz));
	blocksz = 1 << (31 - lz);
	DPRINTF(("blocksz = %u\n", blocksz));

	/* ...but no more than the buffer. */
	if (blocksz > dma->bufsz)
		blocksz = dma->bufsz;

	err = sndbuf_resize(dma->buf, dma->bufsz / blocksz, blocksz);
	if (err != 0) {
		DPRINTF(("sndbuf_resize returned %d\n", err));
		return (0);
	}

	if (blocksz == dma->blksz)
		return (dma->blksz);

	/* One slot per block plus branch to 0 plus STOP. */
	err = dbdma_resize_channel(dma->channel, 2 + dma->bufsz / blocksz);
	if (err != 0) {
		DPRINTF(("dbdma_resize_channel returned %d\n", err));
		return (0);
	}

	/* Set the new blocksize. */
	dma->blksz = blocksz;
	aoa_dma_set_program(dma);

	return (dma->blksz);
}

static int
aoa_chan_setformat(kobj_t obj, void *data, u_int32_t format)
{
	DPRINTF(("aoa_chan_setformat: format = %u\n", format));

	if (format != SND_FORMAT(AFMT_S16_BE, 2, 0))
		return (EINVAL);

	return (0);
}

static u_int32_t
aoa_chan_setspeed(kobj_t obj, void *data, u_int32_t speed)
{
	DPRINTF(("aoa_chan_setspeed: speed = %u\n", speed));

	return (44100);
}

static u_int32_t
aoa_chan_getptr(kobj_t obj, void *data)
{
	struct aoa_dma 	 *dma = data;

	if (!dma->running)
		return (0);
	
	return (dma->slot * dma->blksz);
}

static void *
aoa_chan_init(kobj_t obj, void *devinfo, struct snd_dbuf *b, 
	struct pcm_channel *c, int dir)
{
	struct aoa_softc 	*sc = devinfo;
	struct aoa_dma		*dma;
	int 	 		 max_slots, err;

	KASSERT(dir == PCMDIR_PLAY, ("bad dir"));

	dma = aoa_dma_create(sc);
	if (!dma)
		return (NULL);
	dma->pcm = c;
	dma->buf = b;
	dma->reg = sc->sc_odma;

	/* One slot per block, plus branch to 0 plus STOP. */
	max_slots = 2 + dma->bufsz / dma->blksz;
	err = dbdma_allocate_channel(dma->reg, 0, bus_get_dma_tag(sc->sc_dev),
	    max_slots, &dma->channel );
	if (err != 0) {
		aoa_dma_delete(dma);
		return (NULL);
	}

	if (sndbuf_alloc(dma->buf, dma->tag, 0, dma->bufsz) != 0) {
		dbdma_free_channel(dma->channel);
		aoa_dma_delete(dma);
		return (NULL);
	}

	aoa_dma_set_program(dma);

	return (dma);
}

static int
aoa_chan_trigger(kobj_t obj, void *data, int go)
{
	struct aoa_dma 	*dma = data;
	int 		 i;

	switch (go) {
	case PCMTRIG_START:

		/* Start the DMA. */
		dma->running = 1;
		
		dma->slot = 0;
		dbdma_set_current_cmd(dma->channel, dma->slot);

		dbdma_run(dma->channel);

		return (0);

	case PCMTRIG_STOP:
	case PCMTRIG_ABORT:
		
		mtx_lock(&dma->mutex);

		dma->running = 0;

		/* Make it branch to the STOP command. */
		dbdma_set_device_status(dma->channel, 1 << 0, 1 << 0);

		/* XXX should wait for DBDMA_ACTIVE to clear. */
		DELAY(40000);

		/* Reset the DMA. */
		dbdma_stop(dma->channel);
		dbdma_set_device_status(dma->channel, 1 << 0, 0);

		for (i = 0; i < dma->slots; ++i)
			dbdma_clear_cmd_status(dma->channel, i);

		mtx_unlock(&dma->mutex);

		return (0);
	}

	return (0);
}

static int
aoa_chan_free(kobj_t obj, void *data)
{
	struct aoa_dma 	*dma = data;

	sndbuf_free(dma->buf);
	dbdma_free_channel(dma->channel);
	aoa_dma_delete(dma);

	return (0);
}

void 
aoa_interrupt(void *xsc)
{
	struct aoa_softc	*sc = xsc;
	struct aoa_dma		*dma;

	if (!(dma = sc->sc_intrp) || !dma->running)
		return;

	mtx_lock(&dma->mutex);

	while (dbdma_get_cmd_status(dma->channel, dma->slot)) {

		dbdma_clear_cmd_status(dma->channel, dma->slot);
		dma->slot = (dma->slot + 1) % dma->slots;

		mtx_unlock(&dma->mutex);
		chn_intr(dma->pcm);
		mtx_lock(&dma->mutex);
	}

	mtx_unlock(&dma->mutex);
}

static u_int32_t sc_fmt[] = {
	SND_FORMAT(AFMT_S16_BE, 2, 0),
	0
};
static struct pcmchan_caps aoa_caps = {44100, 44100, sc_fmt, 0};

static struct pcmchan_caps *
aoa_chan_getcaps(kobj_t obj, void *data)
{
	return (&aoa_caps);
}

static kobj_method_t aoa_chan_methods[] = {
	KOBJMETHOD(channel_init, 	aoa_chan_init),
	KOBJMETHOD(channel_free, 	aoa_chan_free),
	KOBJMETHOD(channel_setformat, 	aoa_chan_setformat),
	KOBJMETHOD(channel_setspeed, 	aoa_chan_setspeed),
	KOBJMETHOD(channel_setblocksize,aoa_chan_setblocksize),
	KOBJMETHOD(channel_trigger,	aoa_chan_trigger),
	KOBJMETHOD(channel_getptr,	aoa_chan_getptr),
	KOBJMETHOD(channel_getcaps,	aoa_chan_getcaps),
	KOBJMETHOD_END
};
CHANNEL_DECLARE(aoa_chan);

int
aoa_attach(void *xsc)
{
	char status[SND_STATUSLEN];
	struct aoa_softc *sc;
	device_t self;
	int err;

	sc = xsc;
	self = sc->sc_dev;

	if (pcm_register(self, sc, 1, 0))
		return (ENXIO);

	err = pcm_getbuffersize(self, AOA_BUFFER_SIZE, AOA_BUFFER_SIZE,
	    AOA_BUFFER_SIZE);
	DPRINTF(("pcm_getbuffersize returned %d\n", err));

	pcm_addchan(self, PCMDIR_PLAY, &aoa_chan_class, sc);

	snprintf(status, sizeof(status), "at %s", ofw_bus_get_name(self)); 
	pcm_setstatus(self, status);

	return (0);
}