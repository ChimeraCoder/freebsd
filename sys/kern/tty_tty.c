
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
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/proc.h>
#include <sys/sx.h>
#include <sys/vnode.h>

#include <fs/devfs/devfs.h>
#include <fs/devfs/devfs_int.h>

static	d_open_t	cttyopen;

static struct cdevsw ctty_cdevsw = {
	.d_version =	D_VERSION,
	.d_open =	cttyopen,
	.d_name =	"ctty",
};

static struct cdev *ctty;

static	int
cttyopen(struct cdev *dev, int flag, int mode, struct thread *td)
{

	return (ENXIO);
}

static void
ctty_clone(void *arg, struct ucred *cred, char *name, int namelen,
    struct cdev **dev)
{

	if (*dev != NULL)
		return;
	if (strcmp(name, "tty"))
		return;
	sx_sunlock(&clone_drain_lock);
	sx_slock(&proctree_lock);
	sx_slock(&clone_drain_lock);
	dev_lock();
	if (!(curthread->td_proc->p_flag & P_CONTROLT))
		*dev = ctty;
	else if (curthread->td_proc->p_session->s_ttyvp == NULL)
		*dev = ctty;
	else if (curthread->td_proc->p_session->s_ttyvp->v_type == VBAD ||
	    curthread->td_proc->p_session->s_ttyvp->v_rdev == NULL) {
		/* e.g. s_ttyvp was revoked */
		*dev = ctty;
	} else
		*dev = curthread->td_proc->p_session->s_ttyvp->v_rdev;
	dev_refl(*dev);
	dev_unlock();
	sx_sunlock(&proctree_lock);
}

static void
ctty_drvinit(void *unused)
{

	EVENTHANDLER_REGISTER(dev_clone, ctty_clone, 0, 1000);
	ctty = make_dev_credf(MAKEDEV_ETERNAL, &ctty_cdevsw, 0, NULL, UID_ROOT,
	    GID_WHEEL, 0666, "ctty");
}

SYSINIT(cttydev,SI_SUB_DRIVERS,SI_ORDER_MIDDLE,ctty_drvinit,NULL);