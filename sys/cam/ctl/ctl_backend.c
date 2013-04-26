
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
#include <sys/types.h>
#include <sys/malloc.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/condvar.h>
#include <sys/queue.h>
#include <sys/sysctl.h>

#include <cam/scsi/scsi_all.h>
#include <cam/scsi/scsi_da.h>
#include <cam/ctl/ctl_io.h>
#include <cam/ctl/ctl.h>
#include <cam/ctl/ctl_frontend.h>
#include <cam/ctl/ctl_backend.h>
#include <cam/ctl/ctl_frontend_internal.h>
#include <cam/ctl/ctl_ioctl.h>
#include <cam/ctl/ctl_ha.h>
#include <cam/ctl/ctl_private.h>
#include <cam/ctl/ctl_debug.h>

extern struct ctl_softc *control_softc;

int
ctl_backend_register(struct ctl_backend_driver *be)
{
	struct ctl_softc *ctl_softc;
	struct ctl_backend_driver *be_tmp;

	ctl_softc = control_softc;

	mtx_lock(&ctl_softc->ctl_lock);
	/*
	 * Sanity check, make sure this isn't a duplicate registration.
	 */
	STAILQ_FOREACH(be_tmp, &ctl_softc->be_list, links) {
		if (strcmp(be_tmp->name, be->name) == 0) {
			mtx_unlock(&ctl_softc->ctl_lock);
			return (-1);
		}
	}
	mtx_unlock(&ctl_softc->ctl_lock);

	/*
	 * Call the backend's initialization routine.
	 */
	be->init();

	mtx_lock(&ctl_softc->ctl_lock);
	
	STAILQ_INSERT_TAIL(&ctl_softc->be_list, be, links);

	ctl_softc->num_backends++;

	/*
	 * Don't want to increment the usage count for internal consumers,
	 * we won't be able to unload otherwise.
	 */
	/* XXX KDM find a substitute for this? */
#if 0
	if ((be->flags & CTL_BE_FLAG_INTERNAL) == 0)
		MOD_INC_USE_COUNT;
#endif

#ifdef CS_BE_CONFIG_MOVE_DONE_IS_NOT_USED
	be->config_move_done = ctl_config_move_done;
#endif
	/* XXX KDM fix this! */
	be->num_luns = 0;
#if 0
	atomic_set(&be->num_luns, 0);
#endif

	mtx_unlock(&ctl_softc->ctl_lock);

	return (0);
}

int
ctl_backend_deregister(struct ctl_backend_driver *be)
{
	struct ctl_softc *ctl_softc;

	ctl_softc = control_softc;

	mtx_lock(&ctl_softc->ctl_lock);

#if 0
	if (atomic_read(&be->num_luns) != 0) {
#endif
	/* XXX KDM fix this! */
	if (be->num_luns != 0) {
		mtx_unlock(&ctl_softc->ctl_lock);
		return (-1);
	}

	STAILQ_REMOVE(&ctl_softc->be_list, be, ctl_backend_driver, links);

	ctl_softc->num_backends--;

	/* XXX KDM find a substitute for this? */
#if 0
	if ((be->flags & CTL_BE_FLAG_INTERNAL) == 0)
		MOD_DEC_USE_COUNT;
#endif

	mtx_unlock(&ctl_softc->ctl_lock);

	return (0);
}

struct ctl_backend_driver *
ctl_backend_find(char *backend_name)
{
	struct ctl_softc *ctl_softc;
	struct ctl_backend_driver *be_tmp;

	ctl_softc = control_softc;

	mtx_lock(&ctl_softc->ctl_lock);

	STAILQ_FOREACH(be_tmp, &ctl_softc->be_list, links) {
		if (strcmp(be_tmp->name, backend_name) == 0) {
			mtx_unlock(&ctl_softc->ctl_lock);
			return (be_tmp);
		}
	}

	mtx_unlock(&ctl_softc->ctl_lock);

	return (NULL);
}

/*
 * vim: ts=8
 */