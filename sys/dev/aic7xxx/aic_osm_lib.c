
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

static void	aic_recovery_thread(void *arg);

void
aic_set_recoveryscb(struct aic_softc *aic, struct scb *scb)
{

	if ((scb->flags & SCB_RECOVERY_SCB) == 0) {
		struct scb *list_scb;

		scb->flags |= SCB_RECOVERY_SCB;

		AIC_SCB_DATA(aic)->recovery_scbs++;

		/*
		 * Go through all of our pending SCBs and remove
		 * any scheduled timeouts for them.  We will reschedule
		 * them after we've successfully fixed this problem.
		 */
		LIST_FOREACH(list_scb, &aic->pending_scbs, pending_links) {
			callout_stop(&scb->io_timer);
		}
	}
}

void
aic_platform_timeout(void *arg)
{
	struct	scb *scb;
	
	scb = (struct scb *)arg; 
	aic_lock(scb->aic_softc);
	aic_timeout(scb);
	aic_unlock(scb->aic_softc);
}

int
aic_spawn_recovery_thread(struct aic_softc *aic)
{
	int error;

	error = aic_kthread_create(aic_recovery_thread, aic,
			       &aic->platform_data->recovery_thread,
			       /*flags*/0, /*altstack*/0, "aic_recovery%d",
			       aic->unit);
	return (error);
}

/*
 * Lock is not held on entry.
 */
void
aic_terminate_recovery_thread(struct aic_softc *aic)
{

	if (aic->platform_data->recovery_thread == NULL) {
		return;
	}
	aic->flags |= AIC_SHUTDOWN_RECOVERY;
	wakeup(aic);
	/*
	 * Sleep on a slightly different location 
	 * for this interlock just for added safety.
	 */
	msleep(aic->platform_data, &aic->platform_data->mtx, PUSER, "thtrm", 0);
}

static void
aic_recovery_thread(void *arg)
{
	struct aic_softc *aic;

	aic = (struct aic_softc *)arg;
	aic_lock(aic);
	for (;;) {
		
		if (LIST_EMPTY(&aic->timedout_scbs) != 0
		 && (aic->flags & AIC_SHUTDOWN_RECOVERY) == 0)
			msleep(aic, &aic->platform_data->mtx, PUSER, "idle", 0);

		if ((aic->flags & AIC_SHUTDOWN_RECOVERY) != 0)
			break;

		aic_recover_commands(aic);
	}
	aic->platform_data->recovery_thread = NULL;
	wakeup(aic->platform_data);
	aic_unlock(aic);
	kproc_exit(0);
}

void
aic_calc_geometry(struct ccb_calc_geometry *ccg, int extended)
{
#if __FreeBSD_version >= 500000
	cam_calc_geometry(ccg, extended);
#else
	uint32_t size_mb;
	uint32_t secs_per_cylinder;

	size_mb = ccg->volume_size / ((1024L * 1024L) / ccg->block_size);
	if (size_mb > 1024 && extended) {
		ccg->heads = 255;
		ccg->secs_per_track = 63;
	} else {
		ccg->heads = 64;
		ccg->secs_per_track = 32;
	}
	secs_per_cylinder = ccg->heads * ccg->secs_per_track;
	ccg->cylinders = ccg->volume_size / secs_per_cylinder;
	ccg->ccb_h.status = CAM_REQ_CMP;
#endif
}