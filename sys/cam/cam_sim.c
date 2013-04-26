
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
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/mutex.h>

#include <cam/cam.h>
#include <cam/cam_ccb.h>
#include <cam/cam_sim.h>
#include <cam/cam_queue.h>
#include <cam/cam_xpt.h>

#define CAM_PATH_ANY (u_int32_t)-1

static MALLOC_DEFINE(M_CAMSIM, "CAM SIM", "CAM SIM buffers");

struct cam_devq *
cam_simq_alloc(u_int32_t max_sim_transactions)
{
	return (cam_devq_alloc(/*size*/0, max_sim_transactions));
}

void
cam_simq_free(struct cam_devq *devq)
{
	cam_devq_free(devq);
}

struct cam_sim *
cam_sim_alloc(sim_action_func sim_action, sim_poll_func sim_poll,
	      const char *sim_name, void *softc, u_int32_t unit,
	      struct mtx *mtx, int max_dev_transactions,
	      int max_tagged_dev_transactions, struct cam_devq *queue)
{
	struct cam_sim *sim;

	if (mtx == NULL)
		return (NULL);

	sim = (struct cam_sim *)malloc(sizeof(struct cam_sim),
	    M_CAMSIM, M_ZERO | M_NOWAIT);

	if (sim == NULL)
		return (NULL);

	sim->sim_action = sim_action;
	sim->sim_poll = sim_poll;
	sim->sim_name = sim_name;
	sim->softc = softc;
	sim->path_id = CAM_PATH_ANY;
	sim->unit_number = unit;
	sim->bus_id = 0;	/* set in xpt_bus_register */
	sim->max_tagged_dev_openings = max_tagged_dev_transactions;
	sim->max_dev_openings = max_dev_transactions;
	sim->flags = 0;
	sim->refcount = 1;
	sim->devq = queue;
	sim->max_ccbs = 8;	/* Reserve for management purposes. */
	sim->mtx = mtx;
	if (mtx == &Giant) {
		sim->flags |= 0;
		callout_init(&sim->callout, 0);
	} else {
		sim->flags |= CAM_SIM_MPSAFE;
		callout_init(&sim->callout, 1);
	}

	SLIST_INIT(&sim->ccb_freeq);
	TAILQ_INIT(&sim->sim_doneq);

	return (sim);
}

void
cam_sim_free(struct cam_sim *sim, int free_devq)
{
	union ccb *ccb;
	int error;

	mtx_assert(sim->mtx, MA_OWNED);
	sim->refcount--;
	if (sim->refcount > 0) {
		error = msleep(sim, sim->mtx, PRIBIO, "simfree", 0);
		KASSERT(error == 0, ("invalid error value for msleep(9)"));
	}

	KASSERT(sim->refcount == 0, ("sim->refcount == 0"));

	while ((ccb = (union ccb *)SLIST_FIRST(&sim->ccb_freeq)) != NULL) {
		SLIST_REMOVE_HEAD(&sim->ccb_freeq, xpt_links.sle);
		xpt_free_ccb(ccb);
	}
	if (free_devq)
		cam_simq_free(sim->devq);
	free(sim, M_CAMSIM);
}

void
cam_sim_release(struct cam_sim *sim)
{
	KASSERT(sim->refcount >= 1, ("sim->refcount >= 1"));
	mtx_assert(sim->mtx, MA_OWNED);

	sim->refcount--;
	if (sim->refcount == 0)
		wakeup(sim);
}

void
cam_sim_hold(struct cam_sim *sim)
{
	KASSERT(sim->refcount >= 1, ("sim->refcount >= 1"));
	mtx_assert(sim->mtx, MA_OWNED);

	sim->refcount++;
}

void
cam_sim_set_path(struct cam_sim *sim, u_int32_t path_id)
{
	sim->path_id = path_id;
}