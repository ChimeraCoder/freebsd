
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

#include <linux/init.h>
#include <linux/errno.h>

#include "mlx4.h"

int mlx4_xrcd_alloc(struct mlx4_dev *dev, u32 *xrcdn)
{
	struct mlx4_priv *priv = mlx4_priv(dev);

	*xrcdn = mlx4_bitmap_alloc(&priv->xrcd_bitmap);
	if (*xrcdn == -1)
		return -ENOMEM;

	return 0;
}
EXPORT_SYMBOL_GPL(mlx4_xrcd_alloc);

void mlx4_xrcd_free(struct mlx4_dev *dev, u32 xrcdn)
{
	mlx4_bitmap_free(&mlx4_priv(dev)->xrcd_bitmap, xrcdn);
}
EXPORT_SYMBOL_GPL(mlx4_xrcd_free);

int __devinit mlx4_init_xrcd_table(struct mlx4_dev *dev)
{
	struct mlx4_priv *priv = mlx4_priv(dev);

	return mlx4_bitmap_init(&priv->xrcd_bitmap, (1 << 16),
				(1 << 16) - 1, dev->caps.reserved_xrcds + 1, 0);
}

void mlx4_cleanup_xrcd_table(struct mlx4_dev *dev)
{
	mlx4_bitmap_cleanup(&mlx4_priv(dev)->xrcd_bitmap);
}