
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
 * Disk I/O routines using Open Firmware
 */

#include <sys/param.h>

#include <netinet/in.h>

#include <machine/stdarg.h>

#include <stand.h>

#include "bootstrap.h"
#include "libofw.h"

static int	ofwd_init(void);
static int	ofwd_strategy(void *devdata, int flag, daddr_t dblk,
		    size_t size, char *buf, size_t *rsize);
static int	ofwd_open(struct open_file *f, ...);
static int	ofwd_close(struct open_file *f);
static int	ofwd_ioctl(struct open_file *f, u_long cmd, void *data);
static void	ofwd_print(int verbose);

struct devsw ofwdisk = {
	"block",
	DEVT_DISK,
	ofwd_init,
	ofwd_strategy,
	ofwd_open,
	ofwd_close,
	ofwd_ioctl,
	ofwd_print
};

/*
 * We're not guaranteed to be able to open a device more than once and there
 * is no OFW standard method to determine whether a device is already opened.
 * Opening a device multiple times simultaneously happens to work with most
 * OFW block device drivers but triggers a trap with at least the driver for
 * the on-board controllers of Sun Fire V100 and Ultra 1.  Upper layers and MI
 * code expect to be able to open a device more than once however.  Given that
 * different partitions of the same device might be opened at the same time as
 * done by ZFS, we can't generally just keep track of the opened devices and
 * reuse the instance handle when asked to open an already opened device.  So
 * the best we can do is to cache the lastly used device path and close and
 * open devices in ofwd_strategy() as needed.
 */
static struct ofw_devdesc *kdp;

static int
ofwd_init(void)
{

	return (0);
}

static int
ofwd_strategy(void *devdata, int flag __unused, daddr_t dblk, size_t size,
    char *buf, size_t *rsize)
{
	struct ofw_devdesc *dp = (struct ofw_devdesc *)devdata;
	daddr_t pos;
	int n;

	if (dp != kdp) {
		if (kdp != NULL) {
#if !defined(__powerpc__)
			OF_close(kdp->d_handle);
#endif
			kdp = NULL;
		}
		if ((dp->d_handle = OF_open(dp->d_path)) == -1)
			return (ENOENT);
		kdp = dp;
	}

	pos = dblk * 512;
	do {
		if (OF_seek(dp->d_handle, pos) < 0)
			return (EIO);
		n = OF_read(dp->d_handle, buf, size);
		if (n < 0 && n != -2)
			return (EIO);
	} while (n == -2);
	*rsize = size;
	return (0);
}

static int
ofwd_open(struct open_file *f, ...)
{
	struct ofw_devdesc *dp;
	va_list vl;

	va_start(vl, f);
	dp = va_arg(vl, struct ofw_devdesc *);
	va_end(vl);

	if (dp != kdp) {
		if (kdp != NULL) {
			OF_close(kdp->d_handle);
			kdp = NULL;
		}
		if ((dp->d_handle = OF_open(dp->d_path)) == -1) {
			printf("%s: Could not open %s\n", __func__,
			    dp->d_path);
			return (ENOENT);
		}
		kdp = dp;
	}
	return (0);
}

static int
ofwd_close(struct open_file *f)
{
	struct ofw_devdesc *dev = f->f_devdata;

	if (dev == kdp) {
#if !defined(__powerpc__)
		OF_close(dev->d_handle);
#endif
		kdp = NULL;
	}
	return (0);
}

static int
ofwd_ioctl(struct open_file *f __unused, u_long cmd __unused,
    void *data __unused)
{

	return (EINVAL);
}

static void
ofwd_print(int verbose __unused)
{

}