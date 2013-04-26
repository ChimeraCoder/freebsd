
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
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/priv.h>
#include <sys/disk.h>
#include <sys/bus.h>
#include <sys/filio.h>

#include <machine/bus.h>
#include <machine/vmparam.h>

/* For use with destroy_dev(9). */
static struct cdev *null_dev;
static struct cdev *zero_dev;

static d_write_t null_write;
static d_ioctl_t null_ioctl;
static d_ioctl_t zero_ioctl;
static d_read_t zero_read;

static struct cdevsw null_cdevsw = {
	.d_version =	D_VERSION,
	.d_read =	(d_read_t *)nullop,
	.d_write =	null_write,
	.d_ioctl =	null_ioctl,
	.d_name =	"null",
};

static struct cdevsw zero_cdevsw = {
	.d_version =	D_VERSION,
	.d_read =	zero_read,
	.d_write =	null_write,
	.d_ioctl =	zero_ioctl,
	.d_name =	"zero",
	.d_flags =	D_MMAP_ANON,
};

/* ARGSUSED */
static int
null_write(struct cdev *dev __unused, struct uio *uio, int flags __unused)
{
	uio->uio_resid = 0;

	return (0);
}

/* ARGSUSED */
static int
null_ioctl(struct cdev *dev __unused, u_long cmd, caddr_t data __unused,
    int flags __unused, struct thread *td)
{
	int error;
	error = 0;

	switch (cmd) {
	case DIOCSKERNELDUMP:
		error = priv_check(td, PRIV_SETDUMPER);
		if (error == 0)
			error = set_dumper(NULL, NULL);
		break;
	case FIONBIO:
		break;
	case FIOASYNC:
		if (*(int *)data != 0)
			error = EINVAL;
		break;
	default:
		error = ENOIOCTL;
	}
	return (error);
}

/* ARGSUSED */
static int
zero_ioctl(struct cdev *dev __unused, u_long cmd, caddr_t data __unused,
	   int flags __unused, struct thread *td)
{
	int error;
	error = 0;

	switch (cmd) {
	case FIONBIO:
		break;
	case FIOASYNC:
		if (*(int *)data != 0)
			error = EINVAL;
		break;
	default:
		error = ENOIOCTL;
	}
	return (error);
}


/* ARGSUSED */
static int
zero_read(struct cdev *dev __unused, struct uio *uio, int flags __unused)
{
	void *zbuf;
	ssize_t len;
	int error = 0;

	KASSERT(uio->uio_rw == UIO_READ,
	    ("Can't be in %s for write", __func__));
	zbuf = __DECONST(void *, zero_region);
	while (uio->uio_resid > 0 && error == 0) {
		len = uio->uio_resid;
		if (len > ZERO_REGION_SIZE)
			len = ZERO_REGION_SIZE;
		error = uiomove(zbuf, len, uio);
	}

	return (error);
}

/* ARGSUSED */
static int
null_modevent(module_t mod __unused, int type, void *data __unused)
{
	switch(type) {
	case MOD_LOAD:
		if (bootverbose)
			printf("null: <null device, zero device>\n");
		null_dev = make_dev_credf(MAKEDEV_ETERNAL_KLD, &null_cdevsw, 0,
		    NULL, UID_ROOT, GID_WHEEL, 0666, "null");
		zero_dev = make_dev_credf(MAKEDEV_ETERNAL_KLD, &zero_cdevsw, 0,
		    NULL, UID_ROOT, GID_WHEEL, 0666, "zero");
		break;

	case MOD_UNLOAD:
		destroy_dev(null_dev);
		destroy_dev(zero_dev);
		break;

	case MOD_SHUTDOWN:
		break;

	default:
		return (EOPNOTSUPP);
	}

	return (0);
}

DEV_MODULE(null, null_modevent, NULL);
MODULE_VERSION(null, 1);