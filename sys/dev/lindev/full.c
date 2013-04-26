
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
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/uio.h>

#include <dev/lindev/lindev.h>

static struct cdev *full_dev;

static d_read_t full_read;
static d_write_t full_write;

static struct cdevsw full_cdevsw = {
	.d_version =	D_VERSION,
	.d_read =	full_read,
	.d_write =	full_write,
	.d_name =	"full",
};

static void *zbuf;

/* ARGSUSED */
static int
full_read(struct cdev *dev __unused, struct uio *uio, int flags __unused)
{
	int error = 0;

	while (uio->uio_resid > 0 && error == 0)
		error = uiomove(zbuf, MIN(uio->uio_resid, PAGE_SIZE), uio);

	return (error);
}

/* ARGSUSED */
static int
full_write(struct cdev *dev __unused, struct uio *uio __unused,
    int flags __unused)
{

	return (ENOSPC);
}

/* ARGSUSED */
int
lindev_modevent_full(module_t mod __unused, int type, void *data __unused)
{

	switch(type) {
	case MOD_LOAD:
		zbuf = (void *)malloc(PAGE_SIZE, M_TEMP, M_WAITOK | M_ZERO);
		full_dev = make_dev(&full_cdevsw, 0, UID_ROOT, GID_WHEEL,
		    0666, "full");
		if (bootverbose)
			printf("full: <full device>\n");
		break;

	case MOD_UNLOAD:
		destroy_dev(full_dev);
		free(zbuf, M_TEMP);
		break;

	case MOD_SHUTDOWN:
		break;

	default:
		return (EOPNOTSUPP);
	}

	return (0);
}