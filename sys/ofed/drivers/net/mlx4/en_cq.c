
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

#include <linux/mlx4/cq.h>
#include <linux/mlx4/qp.h>
#include <linux/mlx4/cmd.h>

static void mlx4_en_cq_event(struct mlx4_cq *cq, enum mlx4_event event)
{
	return;
}


int mlx4_en_create_cq(struct mlx4_en_priv *priv,
		      struct mlx4_en_cq *cq,
		      int entries, int ring, enum cq_type mode)
{
	struct mlx4_en_dev *mdev = priv->mdev;
	int err;

	cq->size = entries;
	cq->tq = taskqueue_create_fast("mlx4_en_que", M_NOWAIT,
	    taskqueue_thread_enqueue, &cq->tq);
	if (mode == RX) {
		cq->buf_size = cq->size * sizeof(struct mlx4_cqe);
		cq->vector   = (ring + priv->port) %
				mdev->dev->caps.num_comp_vectors;
		TASK_INIT(&cq->cq_task, 0, mlx4_en_rx_que, cq);
		taskqueue_start_threads(&cq->tq, 1, PI_NET, "%s rx cq",
		    if_name(priv->dev));
	} else {
		cq->buf_size = sizeof(struct mlx4_cqe);
		cq->vector   = MLX4_LEAST_ATTACHED_VECTOR;
		TASK_INIT(&cq->cq_task, 0, mlx4_en_tx_que, cq);
		taskqueue_start_threads(&cq->tq, 1, PI_NET, "%s tx cq",
		    if_name(priv->dev));
	}

	cq->ring = ring;
	cq->is_tx = mode;
	mtx_init(&cq->lock.m, "mlx4 cq", NULL, MTX_DEF);

	err = mlx4_alloc_hwq_res(mdev->dev, &cq->wqres,
				cq->buf_size, 2 * PAGE_SIZE);
	if (err)
		return err;

	err = mlx4_en_map_buffer(&cq->wqres.buf);
	if (err)
		mlx4_free_hwq_res(mdev->dev, &cq->wqres, cq->buf_size);
	else
		cq->buf = (struct mlx4_cqe *) cq->wqres.buf.direct.buf;

	return err;
}

int mlx4_en_activate_cq(struct mlx4_en_priv *priv, struct mlx4_en_cq *cq)
{
	struct mlx4_en_dev *mdev = priv->mdev;
	int err;

	cq->dev = mdev->pndev[priv->port];
	cq->mcq.set_ci_db  = cq->wqres.db.db;
	cq->mcq.arm_db     = cq->wqres.db.db + 1;
	*cq->mcq.set_ci_db = 0;
	*cq->mcq.arm_db    = 0;
	memset(cq->buf, 0, cq->buf_size);

	if (!cq->is_tx)
		cq->size = priv->rx_ring[cq->ring].actual_size;

	err = mlx4_cq_alloc(mdev->dev, cq->size, &cq->wqres.mtt, &mdev->priv_uar,
			    cq->wqres.db.dma, &cq->mcq, cq->vector, cq->is_tx);
	if (err)
		return err;

	cq->mcq.comp  = cq->is_tx ? mlx4_en_tx_irq : mlx4_en_rx_irq;
	cq->mcq.event = mlx4_en_cq_event;

	if (cq->is_tx) {
		init_timer(&cq->timer);
		cq->timer.function = mlx4_en_poll_tx_cq;
		cq->timer.data = (unsigned long) cq;
	}

	return 0;
}

void mlx4_en_destroy_cq(struct mlx4_en_priv *priv, struct mlx4_en_cq *cq)
{
	struct mlx4_en_dev *mdev = priv->mdev;

	taskqueue_drain(cq->tq, &cq->cq_task);
	taskqueue_free(cq->tq);
	mlx4_en_unmap_buffer(&cq->wqres.buf);
	mlx4_free_hwq_res(mdev->dev, &cq->wqres, cq->buf_size);
	cq->buf_size = 0;
	cq->buf = NULL;
	mtx_destroy(&cq->lock.m);
}

void mlx4_en_deactivate_cq(struct mlx4_en_priv *priv, struct mlx4_en_cq *cq)
{
	struct mlx4_en_dev *mdev = priv->mdev;

	taskqueue_drain(cq->tq, &cq->cq_task);
	if (cq->is_tx)
		del_timer(&cq->timer);

	mlx4_cq_free(mdev->dev, &cq->mcq);
}

/* Set rx cq moderation parameters */
int mlx4_en_set_cq_moder(struct mlx4_en_priv *priv, struct mlx4_en_cq *cq)
{
	return mlx4_cq_modify(priv->mdev->dev, &cq->mcq,
			      cq->moder_cnt, cq->moder_time);
}

int mlx4_en_arm_cq(struct mlx4_en_priv *priv, struct mlx4_en_cq *cq)
{
	mlx4_cq_arm(&cq->mcq, MLX4_CQ_DB_REQ_NOT, priv->mdev->uar_map,
		    &priv->mdev->uar_lock);

	return 0;
}