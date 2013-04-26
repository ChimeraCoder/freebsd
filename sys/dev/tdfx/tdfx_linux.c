
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
#include <sys/capability.h>
#include <sys/file.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/proc.h>
#include <sys/systm.h>

#include <dev/tdfx/tdfx_linux.h>

LINUX_IOCTL_SET(tdfx, LINUX_IOCTL_TDFX_MIN, LINUX_IOCTL_TDFX_MAX);

/*
 * Linux emulation IOCTL for /dev/tdfx
 */
static int
linux_ioctl_tdfx(struct thread *td, struct linux_ioctl_args* args)
{
   int error = 0;
   u_long cmd = args->cmd & 0xffff;

   /* The structure passed to ioctl has two shorts, one int
      and one void*. */
   char d_pio[2*sizeof(short) + sizeof(int) + sizeof(void*)];

   struct file *fp;

   if ((error = fget(td, args->fd, CAP_IOCTL, &fp)) != 0)
	   return (error);
   /* We simply copy the data and send it right to ioctl */
   copyin((caddr_t)args->arg, &d_pio, sizeof(d_pio));
   error = fo_ioctl(fp, cmd, (caddr_t)&d_pio, td->td_ucred, td);
   fdrop(fp, td);
   return error;
}

static int
tdfx_linux_modevent(struct module *mod __unused, int what, void *arg __unused)
{

	switch (what) {
	case MOD_LOAD:
	case MOD_UNLOAD:
		return (0);
	}
	return (EOPNOTSUPP);
}

static moduledata_t tdfx_linux_mod = {
	"tdfx_linux",
	tdfx_linux_modevent,
	0
};

/* As in SYSCALL_MODULE */
DECLARE_MODULE(tdfx_linux, tdfx_linux_mod, SI_SUB_DRIVERS, SI_ORDER_MIDDLE);
MODULE_VERSION(tdfx_linux, 1);
MODULE_DEPEND(tdfx_linux, tdfx, 1, 1, 1);
MODULE_DEPEND(tdfx_linux, linux, 1, 1, 1);