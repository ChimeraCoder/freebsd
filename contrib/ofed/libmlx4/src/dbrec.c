
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

#include "mlx4.h"

struct mlx4_db_page {
	struct mlx4_db_page	       *prev, *next;
	struct mlx4_buf			buf;
	int				num_db;
	int				use_cnt;
	unsigned long			free[0];
};

static const int db_size[] = {
	[MLX4_DB_TYPE_CQ] = 8,
	[MLX4_DB_TYPE_RQ] = 4,
};

static struct mlx4_db_page *__add_page(struct mlx4_context *context,
				       enum mlx4_db_type type)
{
	struct mlx4_db_page *page;
	int ps = to_mdev(context->ibv_ctx.device)->page_size;
	int pp;
	int i;

	pp = ps / db_size[type];

	page = malloc(sizeof *page + pp / 8);
	if (!page)
		return NULL;

	if (mlx4_alloc_buf(&page->buf, ps, ps)) {
		free(page);
		return NULL;
	}

	page->num_db  = pp;
	page->use_cnt = 0;
	for (i = 0; i < pp / (sizeof (long) * 8); ++i)
		page->free[i] = ~0;

	page->prev = NULL;
	page->next = context->db_list[type];
	context->db_list[type] = page;
	if (page->next)
		page->next->prev = page;

	return page;
}

uint32_t *mlx4_alloc_db(struct mlx4_context *context, enum mlx4_db_type type)
{
	struct mlx4_db_page *page;
	uint32_t *db = NULL;
	int i, j;

	pthread_mutex_lock(&context->db_list_mutex);

	for (page = context->db_list[type]; page; page = page->next)
		if (page->use_cnt < page->num_db)
			goto found;

	page = __add_page(context, type);
	if (!page)
		goto out;

found:
	++page->use_cnt;

	for (i = 0; !page->free[i]; ++i)
		/* nothing */;

	j = ffsl(page->free[i]);
	page->free[i] &= ~(1UL << (j - 1));
	db = page->buf.buf + (i * 8 * sizeof (long) + (j - 1)) * db_size[type];

out:
	pthread_mutex_unlock(&context->db_list_mutex);

	return db;
}

void mlx4_free_db(struct mlx4_context *context, enum mlx4_db_type type, uint32_t *db)
{
	struct mlx4_db_page *page;
	uintptr_t ps = to_mdev(context->ibv_ctx.device)->page_size;
	int i;

	pthread_mutex_lock(&context->db_list_mutex);

	for (page = context->db_list[type]; page; page = page->next)
		if (((uintptr_t) db & ~(ps - 1)) == (uintptr_t) page->buf.buf)
			break;

	if (!page)
		goto out;

	i = ((void *) db - page->buf.buf) / db_size[type];
	page->free[i / (8 * sizeof (long))] |= 1UL << (i % (8 * sizeof (long)));

	if (!--page->use_cnt) {
		if (page->prev)
			page->prev->next = page->next;
		else
			context->db_list[type] = page->next;
		if (page->next)
			page->next->prev = page->prev;

		mlx4_free_buf(&page->buf);
		free(page);
	}

out:
	pthread_mutex_unlock(&context->db_list_mutex);
}