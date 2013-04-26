
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
#include <sys/conf.h>
#include <sys/fcntl.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/memrange.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/proc.h>
#include <sys/signalvar.h>
#include <sys/systm.h>
#include <sys/uio.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/memdev.h>

static struct cdev *memdev, *kmemdev;

static struct cdevsw mem_cdevsw = {
	.d_version =	D_VERSION,
	.d_flags =	D_MEM|D_NEEDGIANT,
	.d_open =	memopen,
	.d_read =	memrw,
	.d_write =	memrw,
	.d_ioctl =	memioctl,
	.d_mmap =	memmmap,
	.d_name =	"mem",
};

/* ARGSUSED */
int
memopen(struct cdev *dev __unused, int flags, int fmt __unused,
    struct thread *td)
{
	int error = 0;

	if (flags & FWRITE)
		error = securelevel_gt(td->td_ucred, 0);

	return (error);
}

/* ARGSUSED */
static int
mem_modevent(module_t mod __unused, int type, void *data __unused)
{
	switch(type) {
	case MOD_LOAD:
		if (bootverbose)
			printf("mem: <memory>\n");
		mem_range_init();
		memdev = make_dev(&mem_cdevsw, CDEV_MINOR_MEM,
			UID_ROOT, GID_KMEM, 0640, "mem");
		kmemdev = make_dev(&mem_cdevsw, CDEV_MINOR_KMEM,
			UID_ROOT, GID_KMEM, 0640, "kmem");
		break;

	case MOD_UNLOAD:
		mem_range_destroy();
		destroy_dev(memdev);
		destroy_dev(kmemdev);
		break;

	case MOD_SHUTDOWN:
		break;

	default:
		return(EOPNOTSUPP);

	}

	return (0);
}

DEV_MODULE(mem, mem_modevent, NULL);
MODULE_VERSION(mem, 1);