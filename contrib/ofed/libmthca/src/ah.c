
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

#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>

#include "mthca.h"

struct mthca_ah_page {
	struct mthca_ah_page *prev, *next;
	struct mthca_buf      buf;
	struct ibv_mr 	     *mr;
	int           	      use_cnt;
	unsigned      	      free[0];
};

static struct mthca_ah_page *__add_page(struct mthca_pd *pd, int page_size, int per_page)
{
	struct mthca_ah_page *page;
	int i;

	page = malloc(sizeof *page + per_page * sizeof (int));
	if (!page)
		return NULL;

	if (mthca_alloc_buf(&page->buf, page_size, page_size)) {
		free(page);
		return NULL;
	}

	page->mr = mthca_reg_mr(&pd->ibv_pd, page->buf.buf, page_size, 0);
	if (!page->mr) {
		mthca_free_buf(&page->buf);
		free(page);
		return NULL;
	}

	page->mr->context = pd->ibv_pd.context;

	page->use_cnt = 0;
	for (i = 0; i < per_page; ++i)
		page->free[i] = ~0;

	page->prev = NULL;
	page->next = pd->ah_list;
	pd->ah_list = page;
	if (page->next)
		page->next->prev = page;

	return page;
}

int mthca_alloc_av(struct mthca_pd *pd, struct ibv_ah_attr *attr,
		   struct mthca_ah *ah)
{
	if (mthca_is_memfree(pd->ibv_pd.context)) {
		ah->av = malloc(sizeof *ah->av);
		if (!ah->av)
			return -1;
	} else {
		struct mthca_ah_page *page;
		int ps;
		int pp;
		int i, j;

		ps = to_mdev(pd->ibv_pd.context->device)->page_size;
		pp = ps / (sizeof *ah->av * 8 * sizeof (int));

		pthread_mutex_lock(&pd->ah_mutex);
		for (page = pd->ah_list; page; page = page->next)
			if (page->use_cnt < ps / sizeof *ah->av)
				for (i = 0; i < pp; ++i)
					if (page->free[i])
						goto found;

		page = __add_page(pd, ps, pp);
		if (!page) {
			pthread_mutex_unlock(&pd->ah_mutex);
			return -1;
		}

	found:
		++page->use_cnt;

		for (i = 0, j = -1; i < pp; ++i)
			if (page->free[i]) {
				j = ffs(page->free[i]);
				page->free[i] &= ~(1 << (j - 1));
				ah->av = page->buf.buf +
					(i * 8 * sizeof (int) + (j - 1)) * sizeof *ah->av;
				break;
			}

		ah->key  = page->mr->lkey;
		ah->page = page;

		pthread_mutex_unlock(&pd->ah_mutex);
	}

	memset(ah->av, 0, sizeof *ah->av);

	ah->av->port_pd = htonl(pd->pdn | (attr->port_num << 24));
	ah->av->g_slid  = attr->src_path_bits;
	ah->av->dlid    = htons(attr->dlid);
	ah->av->msg_sr  = (3 << 4) | /* 2K message */
		attr->static_rate;
	ah->av->sl_tclass_flowlabel = htonl(attr->sl << 28);
	if (attr->is_global) {
		ah->av->g_slid |= 0x80;
		/* XXX get gid_table length */
		ah->av->gid_index = (attr->port_num - 1) * 32 +
			attr->grh.sgid_index;
		ah->av->hop_limit = attr->grh.hop_limit;
		ah->av->sl_tclass_flowlabel |=
			htonl((attr->grh.traffic_class << 20) |
				    attr->grh.flow_label);
		memcpy(ah->av->dgid, attr->grh.dgid.raw, 16);
	} else {
		/* Arbel workaround -- low byte of GID must be 2 */
		ah->av->dgid[3] = htonl(2);
	}

	return 0;
}

void mthca_free_av(struct mthca_ah *ah)
{
	if (mthca_is_memfree(ah->ibv_ah.context)) {
		free(ah->av);
	} else {
		struct mthca_pd *pd = to_mpd(ah->ibv_ah.pd);
		struct mthca_ah_page *page;
		int i;

		pthread_mutex_lock(&pd->ah_mutex);

		page = ah->page;
		i = ((void *) ah->av - page->buf.buf) / sizeof *ah->av;
		page->free[i / (8 * sizeof (int))] |= 1 << (i % (8 * sizeof (int)));

		if (!--page->use_cnt) {
			if (page->prev)
				page->prev->next = page->next;
			else
				pd->ah_list = page->next;
			if (page->next)
				page->next->prev = page->prev;

			mthca_dereg_mr(page->mr);
			mthca_free_buf(&page->buf);
			free(page);
		}

		pthread_mutex_unlock(&pd->ah_mutex);
	}
}