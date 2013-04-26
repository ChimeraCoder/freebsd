
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
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/ktr.h>
#include <sys/sf_buf.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/bus.h>

#include <cxgb_include.h>
#include <sys/mvec.h>

#include <vm/vm.h>
#include <vm/vm_page.h>
#include <vm/pmap.h>

#ifdef INVARIANTS
#define M_SANITY m_sanity
#else
#define M_SANITY(a, b)
#endif

int
busdma_map_sg_collapse(bus_dma_tag_t tag, bus_dmamap_t map,
	struct mbuf **m, bus_dma_segment_t *segs, int *nsegs)
{
	struct mbuf *n = *m;
	int seg_count, defragged = 0, err = 0;
	bus_dma_segment_t *psegs;
	
	KASSERT(n->m_pkthdr.len, ("packet has zero header len"));
	if (n->m_pkthdr.len <= PIO_LEN)
		return (0);
retry:
	psegs = segs;
	seg_count = 0;
	if (n->m_next == NULL) {
		busdma_map_mbuf_fast(tag, map, n, segs);
		*nsegs = 1;
		return (0);
	}
#if defined(__i386__) || defined(__amd64__)
	while (n && seg_count < TX_MAX_SEGS) {
		/*
		 * firmware doesn't like empty segments
		 */
		if (__predict_true(n->m_len != 0)) {
			seg_count++;
			busdma_map_mbuf_fast(tag, map, n, psegs);
			psegs++;
		}
		n = n->m_next;
	}
#else
	err = bus_dmamap_load_mbuf_sg(tag, map, *m, segs, &seg_count, 0);
#endif	
	if (seg_count == 0) {
		if (cxgb_debug)
			printf("empty segment chain\n");
		err = EFBIG;
		goto err_out;
	}  else if (err == EFBIG || seg_count >= TX_MAX_SEGS) {
		if (cxgb_debug)
			printf("mbuf chain too long: %d max allowed %d\n",
			    seg_count, TX_MAX_SEGS);
		if (!defragged) {
			n = m_defrag(*m, M_NOWAIT);
			if (n == NULL) {
				err = ENOBUFS;
				goto err_out;
			}
			*m = n;
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
busdma_map_sg_vec(bus_dma_tag_t tag, bus_dmamap_t map,
    struct mbuf *m, bus_dma_segment_t *segs, int *nsegs)
{

	for (*nsegs = 0; m != NULL ; segs++, *nsegs += 1, m = m->m_nextpkt)
		busdma_map_mbuf_fast(tag, map, m, segs);
}