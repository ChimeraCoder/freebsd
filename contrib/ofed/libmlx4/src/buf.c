
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
#include <errno.h>
#include <sys/mman.h>

#include "mlx4.h"

#if !(defined(HAVE_IBV_DONTFORK_RANGE) && defined(HAVE_IBV_DOFORK_RANGE))

/*
 * If libibverbs isn't exporting these functions, then there's no
 * point in doing it here, because the rest of libibverbs isn't going
 * to be fork-safe anyway.
 */
static int ibv_dontfork_range(void *base, size_t size)
{
	return 0;
}

static int ibv_dofork_range(void *base, size_t size)
{
	return 0;
}

#endif /* HAVE_IBV_DONTFORK_RANGE && HAVE_IBV_DOFORK_RANGE */

int mlx4_alloc_buf(struct mlx4_buf *buf, size_t size, int page_size)
{
	int ret;

	buf->length = align(size, page_size);
	buf->buf = mmap(NULL, buf->length, PROT_READ | PROT_WRITE,
			MAP_PRIVATE | MAP_ANON, -1, 0);
	if (buf->buf == MAP_FAILED)
		return errno;

	ret = ibv_dontfork_range(buf->buf, size);
	if (ret)
		munmap(buf->buf, buf->length);

	return ret;
}

void mlx4_free_buf(struct mlx4_buf *buf)
{
	ibv_dofork_range(buf->buf, buf->length);
	munmap(buf->buf, buf->length);
}