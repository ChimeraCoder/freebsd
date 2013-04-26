
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
#include <sys/capability.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/file.h>
#include <sys/proc.h>

#if defined(__amd64__) /* Assume amd64 wants 32 bit Linux */
#include <machine/../linux32/linux.h>
#include <machine/../linux32/linux32_proto.h>
#else
#include <machine/../linux/linux.h>
#include <machine/../linux/linux_proto.h>
#endif
#include <compat/linux/linux_ioctl.h>

/* There are multiple ioctl number ranges that need to be handled */
#define AMR_LINUX_IOCTL_MIN  0x6d00
#define AMR_LINUX_IOCTL_MAX  0x6d01

static linux_ioctl_function_t amr_linux_ioctl;
static struct linux_ioctl_handler amr_linux_handler = {amr_linux_ioctl,
						       AMR_LINUX_IOCTL_MIN,
						       AMR_LINUX_IOCTL_MAX};

SYSINIT  (amr_register,   SI_SUB_KLD, SI_ORDER_MIDDLE,
	  linux_ioctl_register_handler, &amr_linux_handler);
SYSUNINIT(amr_unregister, SI_SUB_KLD, SI_ORDER_MIDDLE,
	  linux_ioctl_unregister_handler, &amr_linux_handler);

static int
amr_linux_modevent(module_t mod, int cmd, void *data)
{
	return (0);
}

DEV_MODULE(amr_linux, amr_linux_modevent, NULL);
MODULE_DEPEND(amr, linux, 1, 1, 1);

static int
amr_linux_ioctl(struct thread *p, struct linux_ioctl_args *args)
{
	struct file *fp;
	int error;

	if ((error = fget(p, args->fd, CAP_IOCTL, &fp)) != 0)
		return (error);
	error = fo_ioctl(fp, args->cmd, (caddr_t)args->arg, p->td_ucred, p);
	fdrop(fp, p);
	return (error);
}