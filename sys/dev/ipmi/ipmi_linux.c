
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
 * Linux ioctl handler for the ipmi device driver
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
#include <sys/ioccom.h>
#include <sys/ipmi.h>

/* There are multiple ioctl number ranges that need to be handled */
#define IPMI_LINUX_IOCTL_MIN  0x690b
#define IPMI_LINUX_IOCTL_MAX  0x6915

/* Linux versions of ioctl's */
#define L_IPMICTL_RECEIVE_MSG_TRUNC       _IOWR(IPMI_IOC_MAGIC, 11, struct ipmi_recv)
#define L_IPMICTL_RECEIVE_MSG             _IOWR(IPMI_IOC_MAGIC, 12, struct ipmi_recv)
#define L_IPMICTL_SEND_COMMAND            _IOW(IPMI_IOC_MAGIC, 13, struct ipmi_req)
#define L_IPMICTL_REGISTER_FOR_CMD        _IOW(IPMI_IOC_MAGIC, 14, struct ipmi_cmdspec)
#define L_IPMICTL_UNREGISTER_FOR_CMD      _IOW(IPMI_IOC_MAGIC, 15, struct ipmi_cmdspec)
#define L_IPMICTL_SET_GETS_EVENTS_CMD     _IOW(IPMI_IOC_MAGIC, 16, int)
#define L_IPMICTL_SET_MY_ADDRESS_CMD      _IOW(IPMI_IOC_MAGIC, 17, unsigned int)
#define L_IPMICTL_GET_MY_ADDRESS_CMD      _IOW(IPMI_IOC_MAGIC, 18, unsigned int)
#define L_IPMICTL_SET_MY_LUN_CMD          _IOW(IPMI_IOC_MAGIC, 19, unsigned int)
#define L_IPMICTL_GET_MY_LUN_CMD          _IOW(IPMI_IOC_MAGIC, 20, unsigned int)

static linux_ioctl_function_t ipmi_linux_ioctl;
static struct linux_ioctl_handler ipmi_linux_handler = {ipmi_linux_ioctl,
						       IPMI_LINUX_IOCTL_MIN,
						       IPMI_LINUX_IOCTL_MAX};

SYSINIT  (ipmi_linux_register,   SI_SUB_KLD, SI_ORDER_MIDDLE,
	  linux_ioctl_register_handler, &ipmi_linux_handler);
SYSUNINIT(ipmi_linux_unregister, SI_SUB_KLD, SI_ORDER_MIDDLE,
	  linux_ioctl_unregister_handler, &ipmi_linux_handler);

static int
ipmi_linux_modevent(module_t mod, int type, void *data)
{
	/* Do we care about any specific load/unload actions? */
	return (0);
}

DEV_MODULE(ipmi_linux, ipmi_linux_modevent, NULL);
MODULE_DEPEND(ipmi_linux, linux, 1, 1, 1);

static int
ipmi_linux_ioctl(struct thread *td, struct linux_ioctl_args *args)
{
	struct file *fp;
	u_long cmd;
	int error;

	if ((error = fget(td, args->fd, CAP_IOCTL, &fp)) != 0)
		return (error);
	cmd = args->cmd;

	switch(cmd) {
	case L_IPMICTL_GET_MY_ADDRESS_CMD:
		cmd = IPMICTL_GET_MY_ADDRESS_CMD;
		break;
	case L_IPMICTL_GET_MY_LUN_CMD:
		cmd = IPMICTL_GET_MY_LUN_CMD;
		break;
	}
	/*
	 * Pass the ioctl off to our standard handler.
	 */
	error = (fo_ioctl(fp, cmd, (caddr_t)args->arg, td->td_ucred, td));
	fdrop(fp, td);
	return (error);
}