
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
#include <sys/bus.h>

#include <machine/bus.h>

#include "common/efx.h"

#include "sfxge.h"

static void
sfxge_dma_cb(void *arg, bus_dma_segment_t *segs, int nseg, int error)
{
	bus_addr_t *addr;

	addr = arg;

	if (error) {
		*addr = 0;
		return;
	}

	*addr = segs[0].ds_addr;
}

int
sfxge_dma_map_sg_collapse(bus_dma_tag_t tag, bus_dmamap_t map,
    struct mbuf **mp, bus_dma_segment_t *segs, int *nsegs, int maxsegs)
{
	bus_dma_segment_t *psegs;
	struct mbuf *m;
	int seg_count;
	int defragged;
	int err;

	m = *mp;
	defragged = err = seg_count = 0;

	KASSERT(m->m_pkthdr.len, ("packet has zero header length"));

retry:
	psegs = segs;
	seg_count = 0;
	if (m->m_next == NULL) {
		sfxge_map_mbuf_fast(tag, map, m, segs);
		*nsegs = 1;
		return (0);
	}
#if defined(__i386__) || defined(__amd64__)
	while (m && seg_count < maxsegs) {
		/*
		 * firmware doesn't like empty segments
		 */
		if (m->m_len != 0) {
			seg_count++;
			sfxge_map_mbuf_fast(tag, map, m, psegs);
			psegs++;
		}
		m = m->m_next;
	}
#else
	err = bus_dmamap_load_mbuf_sg(tag, map, *mp, segs, &seg_count, 0);
#endif
	if (seg_count == 0) {
		err = EFBIG;
		goto err_out;
	} else if (err == EFBIG || seg_count >= maxsegs) {
		if (!defragged) {
			m = m_defrag(*mp, M_NOWAIT);
			if (m == NULL) {
				err = ENOBUFS;
				goto err_out;
			}
			*mp = m;
			defragged = 1;
			goto retry;
		}
		err = EFBIG;
		goto err_out;
	}
	*nsegs = seg_count;

err_out:
	return (err);
}

void
sfxge_dma_free(efsys_mem_t *esmp)
{

	bus_dmamap_unload(esmp->esm_tag, esmp->esm_map);
	bus_dmamem_free(esmp->esm_tag, esmp->esm_base, esmp->esm_map);
	bus_dma_tag_destroy(esmp->esm_tag);

	esmp->esm_addr = 0;
	esmp->esm_base = NULL;
}

int
sfxge_dma_alloc(struct sfxge_softc *sc, bus_size_t len, efsys_mem_t *esmp)
{
	void *vaddr;

	/* Create the child DMA tag. */
	if (bus_dma_tag_create(sc->parent_dma_tag, PAGE_SIZE, 0,
	    MIN(0x3FFFFFFFFFFFUL, BUS_SPACE_MAXADDR), BUS_SPACE_MAXADDR, NULL,
	    NULL, len, 1, len, 0, NULL, NULL, &esmp->esm_tag) != 0) {
		device_printf(sc->dev, "Couldn't allocate txq DMA tag\n");
		return (ENOMEM);
	}

	/* Allocate kernel memory. */
	if (bus_dmamem_alloc(esmp->esm_tag, (void **)&vaddr,
	    BUS_DMA_WAITOK | BUS_DMA_COHERENT | BUS_DMA_ZERO,
	    &esmp->esm_map) != 0) {
		device_printf(sc->dev, "Couldn't allocate DMA memory\n");
		bus_dma_tag_destroy(esmp->esm_tag);
		return (ENOMEM);
	}

	/* Load map into device memory. */
	if (bus_dmamap_load(esmp->esm_tag, esmp->esm_map, vaddr, len,
	    sfxge_dma_cb, &esmp->esm_addr, 0) != 0) {
		device_printf(sc->dev, "Couldn't load DMA mapping\n");
		bus_dmamem_free(esmp->esm_tag, esmp->esm_base, esmp->esm_map);
		bus_dma_tag_destroy(esmp->esm_tag);
		return (ENOMEM);
	}

	/*
	 * The callback gets error information about the mapping
	 * and will have set our vaddr to NULL if something went
	 * wrong.
	 */
	if (vaddr == NULL)
		return (ENOMEM);

	esmp->esm_base = vaddr;

	return (0);
}

void
sfxge_dma_fini(struct sfxge_softc *sc)
{

	bus_dma_tag_destroy(sc->parent_dma_tag);
}

int
sfxge_dma_init(struct sfxge_softc *sc)
{

	/* Create the parent dma tag. */
	if (bus_dma_tag_create(bus_get_dma_tag(sc->dev),	/* parent */
	    1, 0,			/* algnmnt, boundary */
	    BUS_SPACE_MAXADDR,		/* lowaddr */
	    BUS_SPACE_MAXADDR,		/* highaddr */
	    NULL, NULL,			/* filter, filterarg */
	    BUS_SPACE_MAXSIZE_32BIT,	/* maxsize */
	    BUS_SPACE_UNRESTRICTED,	/* nsegments */
	    BUS_SPACE_MAXSIZE_32BIT,	/* maxsegsize */
	    0,				/* flags */
	    NULL, NULL,			/* lock, lockarg */
	    &sc->parent_dma_tag)) {
		device_printf(sc->dev, "Cannot allocate parent DMA tag\n");
		return (ENOMEM);
	}

	return (0);
}