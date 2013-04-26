
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

/*
 * Linux ioctl handler for the aac device driver
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/capability.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/file.h>
#include <sys/proc.h>
#ifdef __amd64__
#include <machine/../linux32/linux.h>
#include <machine/../linux32/linux32_proto.h>
#else
#include <machine/../linux/linux.h>
#include <machine/../linux/linux_proto.h>
#endif
#include <compat/linux/linux_ioctl.h>

/* There are multiple ioctl number ranges that need to be handled */
#define AAC_LINUX_IOCTL_MIN  0x0000
#define AAC_LINUX_IOCTL_MAX  0x21ff

static linux_ioctl_function_t aac_linux_ioctl;
static struct linux_ioctl_handler aac_linux_handler = {aac_linux_ioctl,
						       AAC_LINUX_IOCTL_MIN,
						       AAC_LINUX_IOCTL_MAX};

SYSINIT  (aac_linux_register,   SI_SUB_KLD, SI_ORDER_MIDDLE,
	  linux_ioctl_register_handler, &aac_linux_handler);
SYSUNINIT(aac_linux_unregister, SI_SUB_KLD, SI_ORDER_MIDDLE,
	  linux_ioctl_unregister_handler, &aac_linux_handler);

static int
aac_linux_modevent(module_t mod, int type, void *data)
{
	/* Do we care about any specific load/unload actions? */
	return (0);
}

DEV_MODULE(aac_linux, aac_linux_modevent, NULL);
MODULE_DEPEND(aac_linux, linux, 1, 1, 1);

static int
aac_linux_ioctl(struct thread *td, struct linux_ioctl_args *args)
{
	struct file *fp;
	u_long cmd;
	int error;

	if ((error = fget(td, args->fd, CAP_IOCTL, &fp)) != 0)
		return (error);
	cmd = args->cmd;

	/*
	 * Pass the ioctl off to our standard handler.
	 */
	error = (fo_ioctl(fp, cmd, (caddr_t)args->arg, td->td_ucred, td));
	fdrop(fp, td);
	return (error);
}