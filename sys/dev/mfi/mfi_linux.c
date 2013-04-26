
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
#include <machine/bus.h>

#if defined(__amd64__) /* Assume amd64 wants 32 bit Linux */
#include <machine/../linux32/linux.h>
#include <machine/../linux32/linux32_proto.h>
#else
#include <machine/../linux/linux.h>
#include <machine/../linux/linux_proto.h>
#endif
#include <compat/linux/linux_ioctl.h>
#include <compat/linux/linux_util.h>

#include <dev/mfi/mfireg.h>
#include <dev/mfi/mfi_ioctl.h>

/* There are multiple ioctl number ranges that need to be handled */
#define MFI_LINUX_IOCTL_MIN  0x4d00
#define MFI_LINUX_IOCTL_MAX  0x4d04

static linux_ioctl_function_t mfi_linux_ioctl;
static struct linux_ioctl_handler mfi_linux_handler = {mfi_linux_ioctl,
						       MFI_LINUX_IOCTL_MIN,
						       MFI_LINUX_IOCTL_MAX};

SYSINIT  (mfi_register,   SI_SUB_KLD, SI_ORDER_MIDDLE,
	  linux_ioctl_register_handler, &mfi_linux_handler);
SYSUNINIT(mfi_unregister, SI_SUB_KLD, SI_ORDER_MIDDLE,
	  linux_ioctl_unregister_handler, &mfi_linux_handler);

static struct linux_device_handler mfi_device_handler =
	{ "mfi", "megaraid_sas", "mfi0", "megaraid_sas_ioctl_node", -1, 0, 1};

SYSINIT  (mfi_register2,   SI_SUB_KLD, SI_ORDER_MIDDLE,
	  linux_device_register_handler, &mfi_device_handler);
SYSUNINIT(mfi_unregister2, SI_SUB_KLD, SI_ORDER_MIDDLE,
	  linux_device_unregister_handler, &mfi_device_handler);

static int
mfi_linux_modevent(module_t mod, int cmd, void *data)
{
	return (0);
}

DEV_MODULE(mfi_linux, mfi_linux_modevent, NULL);
MODULE_DEPEND(mfi, linux, 1, 1, 1);

static int
mfi_linux_ioctl(struct thread *p, struct linux_ioctl_args *args)
{
	struct file *fp;
	int error;
	u_long cmd = args->cmd;

	switch (cmd) {
	case MFI_LINUX_CMD:
		cmd = MFI_LINUX_CMD_2;
		break;
	case MFI_LINUX_SET_AEN:
		cmd = MFI_LINUX_SET_AEN_2;
		break;
	}

	if ((error = fget(p, args->fd, CAP_IOCTL, &fp)) != 0)
		return (error);
	error = fo_ioctl(fp, cmd, (caddr_t)args->arg, p->td_ucred, p);
	fdrop(fp, p);
	return (error);
}