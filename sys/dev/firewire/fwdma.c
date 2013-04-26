
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

#ifdef __FreeBSD__
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");
#endif

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#if defined(__FreeBSD__) && __FreeBSD_version >= 501102 
#include <sys/lock.h>
#include <sys/mutex.h>
#endif

#include <sys/bus.h>
#include <machine/bus.h>

#ifdef __DragonFly__
#include <bus/firewire/firewire.h>
#include <bus/firewire/firewirereg.h>
#include <bus/firewire/fwdma.h>
#else
#include <dev/firewire/firewire.h>
#include <dev/firewire/firewirereg.h>
#include <dev/firewire/fwdma.h>
#endif

static void
fwdma_map_cb(void *arg, bus_dma_segment_t *segs, int nseg, int error)
{
	bus_addr_t *baddr;

	if (error)
		printf("fwdma_map_cb: error=%d\n", error);
	baddr = (bus_addr_t *)arg;
	*baddr = segs->ds_addr;
}

void *
fwdma_malloc(struct firewire_comm *fc, int alignment, bus_size_t size,
	struct fwdma_alloc *dma, int flag)
{
	int err;

	dma->v_addr = NULL;
	err = bus_dma_tag_create(
		/*parent*/ fc->dmat,
		/*alignment*/ alignment,
		/*boundary*/ 0,
		/*lowaddr*/ BUS_SPACE_MAXADDR_32BIT,
		/*highaddr*/ BUS_SPACE_MAXADDR,
		/*filter*/NULL, /*filterarg*/NULL,
		/*maxsize*/ size,
		/*nsegments*/ 1,
		/*maxsegsz*/ BUS_SPACE_MAXSIZE_32BIT,
		/*flags*/ BUS_DMA_ALLOCNOW,
#if defined(__FreeBSD__) && __FreeBSD_version >= 501102 
		/*lockfunc*/busdma_lock_mutex,
		/*lockarg*/FW_GMTX(fc),
#endif
		&dma->dma_tag);
	if (err) {
		printf("fwdma_malloc: failed(1)\n");
		return(NULL);
	}

	err = bus_dmamem_alloc(dma->dma_tag, &dma->v_addr,
		flag, &dma->dma_map);
	if (err) {
		printf("fwdma_malloc: failed(2)\n");
		/* XXX destroy tag */
		return(NULL);
	}

	bus_dmamap_load(dma->dma_tag, dma->dma_map, dma->v_addr,
		size, fwdma_map_cb, &dma->bus_addr, /*flags*/0);

	return(dma->v_addr);
}

void
fwdma_free(struct firewire_comm *fc, struct fwdma_alloc *dma)
{
        bus_dmamap_unload(dma->dma_tag, dma->dma_map);
	bus_dmamem_free(dma->dma_tag, dma->v_addr, dma->dma_map);
	bus_dma_tag_destroy(dma->dma_tag);
}


void *
fwdma_malloc_size(bus_dma_tag_t dmat, bus_dmamap_t *dmamap,
	bus_size_t size, bus_addr_t *bus_addr, int flag)
{
	void *v_addr;

	if (bus_dmamem_alloc(dmat, &v_addr, flag, dmamap)) {
		printf("fwdma_malloc_size: failed(1)\n");
		return(NULL);
	}
	bus_dmamap_load(dmat, *dmamap, v_addr, size,
			fwdma_map_cb, bus_addr, /*flags*/0);
	return(v_addr);
}

void
fwdma_free_size(bus_dma_tag_t dmat, bus_dmamap_t dmamap,
		void *vaddr, bus_size_t size)
{
	bus_dmamap_unload(dmat, dmamap);
	bus_dmamem_free(dmat, vaddr, dmamap);
} 

/*
 * Allocate multisegment dma buffers
 * each segment size is eqaul to ssize except last segment.
 */
struct fwdma_alloc_multi *
fwdma_malloc_multiseg(struct firewire_comm *fc, int alignment,
		int esize, int n, int flag)
{
	struct fwdma_alloc_multi *am;
	struct fwdma_seg *seg;
	bus_size_t ssize;
	int nseg;

	if (esize > PAGE_SIZE) {
		/* round up to PAGE_SIZE */
		esize = ssize = roundup2(esize, PAGE_SIZE);
		nseg = n;
	} else {
		/* allocate PAGE_SIZE segment for small elements */
		ssize = rounddown(PAGE_SIZE, esize);
		nseg = howmany(n, ssize / esize);
	}
	am = (struct fwdma_alloc_multi *)malloc(sizeof(struct fwdma_alloc_multi)
			+ sizeof(struct fwdma_seg)*nseg, M_FW, M_WAITOK);
	if (am == NULL) {
		printf("fwdma_malloc_multiseg: malloc failed\n");
		return(NULL);
	}
	am->ssize = ssize;
	am->esize = esize;
	am->nseg = 0;
	if (bus_dma_tag_create(
			/*parent*/ fc->dmat,
			/*alignment*/ alignment,
			/*boundary*/ 0,
			/*lowaddr*/ BUS_SPACE_MAXADDR_32BIT,
			/*highaddr*/ BUS_SPACE_MAXADDR,
			/*filter*/NULL, /*filterarg*/NULL,
			/*maxsize*/ ssize,
			/*nsegments*/ 1,
			/*maxsegsz*/ BUS_SPACE_MAXSIZE_32BIT,
			/*flags*/ BUS_DMA_ALLOCNOW,
#if defined(__FreeBSD__) && __FreeBSD_version >= 501102
			/*lockfunc*/busdma_lock_mutex,
			/*lockarg*/FW_GMTX(fc),
#endif
			&am->dma_tag)) {
		printf("fwdma_malloc_multiseg: tag_create failed\n");
		free(am, M_FW);
		return(NULL);
	}

#if 0
#if defined(__DragonFly__) || __FreeBSD_version < 500000
	printf("malloc_multi: ssize=%d nseg=%d\n", ssize, nseg);
#else
	printf("malloc_multi: ssize=%td nseg=%d\n", ssize, nseg);
#endif
#endif
	for (seg = &am->seg[0]; nseg --; seg ++) {
		seg->v_addr = fwdma_malloc_size(am->dma_tag, &seg->dma_map,
			ssize, &seg->bus_addr, flag);
		if (seg->v_addr == NULL) {
			printf("fwdma_malloc_multi: malloc_size failed %d\n",
				am->nseg);
			fwdma_free_multiseg(am);
			return(NULL);
		}
		am->nseg++;
	}
	return(am);
}

void
fwdma_free_multiseg(struct fwdma_alloc_multi *am)
{
	struct fwdma_seg *seg;

	for (seg = &am->seg[0]; am->nseg --; seg ++) {
		fwdma_free_size(am->dma_tag, seg->dma_map,
			seg->v_addr, am->ssize);
	}
	bus_dma_tag_destroy(am->dma_tag);
	free(am, M_FW);
}