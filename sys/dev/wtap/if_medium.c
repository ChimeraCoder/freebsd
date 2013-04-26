
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
#include "if_wtapvar.h"
#include "if_medium.h"

void
init_medium(struct wtap_medium *md)
{

	DWTAP_PRINTF("%s\n", __func__);
	STAILQ_INIT(&md->md_pktbuf);
	mtx_init(&md->md_mtx, "wtap_medium mtx", NULL, MTX_DEF | MTX_RECURSE);

	/* Event handler for sending packets between wtaps */
	struct eventhandler *eh = (struct eventhandler *)
	    malloc(sizeof(struct eventhandler), M_WTAP, M_NOWAIT | M_ZERO);
	eh->tq = taskqueue_create("wtap_tx_taskq",  M_NOWAIT | M_ZERO,
	    taskqueue_thread_enqueue, &eh->tq);
	taskqueue_start_threads(&eh->tq, 1, PI_NET, "%s taskq", "wtap_medium");
	md->tx_handler = eh;
	/* Mark medium closed by default */
	md->open = 0;
}

void
deinit_medium(struct wtap_medium *md)
{

	DWTAP_PRINTF("%s\n", __func__);
	taskqueue_free(md->tx_handler->tq);
	free(md->tx_handler, M_WTAP);
}

int
medium_transmit(struct wtap_medium *md, int id, struct mbuf*m)
{

	mtx_lock(&md->md_mtx);
	if (md->open == 0){
		DWTAP_PRINTF("[%d] dropping m=%p\n", id, m);
		m_free(m);
		mtx_unlock(&md->md_mtx);
		return 0;
	}

	DWTAP_PRINTF("[%d] transmiting m=%p\n", id, m);
	struct packet *p = (struct packet *)malloc(sizeof(struct packet),
	    M_WTAP_PACKET, M_ZERO | M_NOWAIT);
	p->id = id;
	p->m = m;

	STAILQ_INSERT_TAIL(&md->md_pktbuf, p, pf_list);
	taskqueue_enqueue(md->tx_handler->tq, &md->tx_handler->proc);
	mtx_unlock(&md->md_mtx);

      return 0;
}

struct packet *
medium_get_next_packet(struct wtap_medium *md)
{
	struct packet *p;

	mtx_lock(&md->md_mtx);
	p = STAILQ_FIRST(&md->md_pktbuf);
	if (p == NULL){
		mtx_unlock(&md->md_mtx);
		return NULL;
	}

	STAILQ_REMOVE_HEAD(&md->md_pktbuf, pf_list);
	mtx_unlock(&md->md_mtx);
	return p;
}

void
medium_open(struct wtap_medium *md)
{

	mtx_lock(&md->md_mtx);
	md->open = 1;
	mtx_unlock(&md->md_mtx);
}

void
medium_close(struct wtap_medium *md)
{

	mtx_lock(&md->md_mtx);
	md->open = 0;
	mtx_unlock(&md->md_mtx);
}