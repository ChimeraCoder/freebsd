
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

#include "mlx4_en.h"

#include <linux/kernel.h>
#include <linux/ethtool.h>
#include <linux/netdevice.h>
#include <linux/delay.h>
#include <linux/mlx4/driver.h>


static int mlx4_en_test_registers(struct mlx4_en_priv *priv)
{
	return mlx4_cmd(priv->mdev->dev, 0, 0, 0, MLX4_CMD_HW_HEALTH_CHECK,
			MLX4_CMD_TIME_CLASS_A);
}

static int mlx4_en_test_loopback_xmit(struct mlx4_en_priv *priv)
{
	struct mbuf *mb;
	struct ethhdr *ethh;
	unsigned char *packet;
	unsigned int packet_size = MLX4_LOOPBACK_TEST_PAYLOAD;
	unsigned int i;
	int err;


	/* build the pkt before xmit */
	mb = netdev_alloc_mb(priv->dev, MLX4_LOOPBACK_TEST_PAYLOAD + ETH_HLEN + NET_IP_ALIGN);
	if (!mb) {
		en_err(priv, "-LOOPBACK_TEST_XMIT- failed to create mb for xmit\n");
		return -ENOMEM;
	}
	mb_reserve(mb, NET_IP_ALIGN);

	ethh = (struct ethhdr *)mb_put(mb, sizeof(struct ethhdr));
	packet	= (unsigned char *)mb_put(mb, packet_size);
	memcpy(ethh->h_dest, priv->dev->dev_addr, ETH_ALEN);
	memset(ethh->h_source, 0, ETH_ALEN);
	ethh->h_proto = htons(ETH_P_ARP);
	mb_set_mac_header(mb, 0);
	for (i = 0; i < packet_size; ++i)	/* fill our packet */
		packet[i] = (unsigned char)(i & 0xff);

	/* xmit the pkt */
	err = mlx4_en_xmit(mb, priv->dev);
	return err;
}

static int mlx4_en_test_loopback(struct mlx4_en_priv *priv)
{
	u32 loopback_ok = 0;
	int i;


        priv->loopback_ok = 0;
	priv->validate_loopback = 1;

	/* xmit */
	if (mlx4_en_test_loopback_xmit(priv)) {
		en_err(priv, "Transmitting loopback packet failed\n");
		goto mlx4_en_test_loopback_exit;
	}

	/* polling for result */
	for (i = 0; i < MLX4_EN_LOOPBACK_RETRIES; ++i) {
		msleep(MLX4_EN_LOOPBACK_TIMEOUT);
		if (priv->loopback_ok) {
			loopback_ok = 1;
			break;
		}
	}
	if (!loopback_ok)
		en_err(priv, "Loopback packet didn't arrive\n");

mlx4_en_test_loopback_exit:

	priv->validate_loopback = 0;
	return (!loopback_ok);
}


static int mlx4_en_test_link(struct mlx4_en_priv *priv)
{
	if (mlx4_en_QUERY_PORT(priv->mdev, priv->port))
		return -ENOMEM;
	if (priv->port_state.link_state == 1)
		return 0;
	else
		return 1;
}

static int mlx4_en_test_speed(struct mlx4_en_priv *priv)
{

	if (mlx4_en_QUERY_PORT(priv->mdev, priv->port))
		return -ENOMEM;

	/* The device currently only supports 10G speed */
	if (priv->port_state.link_speed != SPEED_10000)
		return priv->port_state.link_speed;
	return 0;
}


void mlx4_en_ex_selftest(struct net_device *dev, u32 *flags, u64 *buf)
{
	struct mlx4_en_priv *priv = netdev_priv(dev);
	struct mlx4_en_dev *mdev = priv->mdev;
	struct mlx4_en_tx_ring *tx_ring;
	int i, carrier_ok;

	memset(buf, 0, sizeof(u64) * MLX4_EN_NUM_SELF_TEST);

	if (*flags & ETH_TEST_FL_OFFLINE) {
		/* disable the interface */
		carrier_ok = netif_carrier_ok(dev);

		netif_carrier_off(dev);
retry_tx:
		/* Wait untill all tx queues are empty.
		 * there should not be any additional incoming traffic
		 * since we turned the carrier off */
		msleep(200);
		for (i = 0; i < priv->tx_ring_num && carrier_ok; i++) {
			tx_ring = &priv->tx_ring[i];
			if (tx_ring->prod != (tx_ring->cons + tx_ring->last_nr_txbb))
				goto retry_tx;
		}

		if (priv->mdev->dev->caps.loopback_support){
			buf[3] = mlx4_en_test_registers(priv);
			buf[4] = mlx4_en_test_loopback(priv);
		}

		if (carrier_ok)
			netif_carrier_on(dev);

	}
	buf[0] = mlx4_test_interrupts(mdev->dev);
	buf[1] = mlx4_en_test_link(priv);
	buf[2] = mlx4_en_test_speed(priv);

	for (i = 0; i < MLX4_EN_NUM_SELF_TEST; i++) {
		if (buf[i])
			*flags |= ETH_TEST_FL_FAILED;
	}
}