
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
 * Userboot disk image handling.
 */

#include <sys/disk.h>
#include <stand.h>
#include <stdarg.h>
#include <bootstrap.h>

#include "disk.h"
#include "libuserboot.h"

struct userdisk_info {
	uint64_t	mediasize;
	uint16_t	sectorsize;
};

int userboot_disk_maxunit = 0;

static int userdisk_maxunit = 0;
static struct userdisk_info	*ud_info;

static int	userdisk_init(void);
static void	userdisk_cleanup(void);
static int	userdisk_strategy(void *devdata, int flag, daddr_t dblk,
		    size_t size, char *buf, size_t *rsize);
static int	userdisk_open(struct open_file *f, ...);
static int	userdisk_close(struct open_file *f);
static int	userdisk_ioctl(struct open_file *f, u_long cmd, void *data);
static void	userdisk_print(int verbose);

struct devsw userboot_disk = {
	"disk",
	DEVT_DISK,
	userdisk_init,
	userdisk_strategy,
	userdisk_open,
	userdisk_close,
	userdisk_ioctl,
	userdisk_print,
	userdisk_cleanup
};

/*
 * Initialize userdisk_info structure for each disk.
 */
static int
userdisk_init(void)
{
	off_t mediasize;
	u_int sectorsize;
	int i;

	userdisk_maxunit = userboot_disk_maxunit;
	if (userdisk_maxunit > 0) {
		ud_info = malloc(sizeof(*ud_info) * userdisk_maxunit);
		if (ud_info == NULL)
			return (ENOMEM);
		for (i = 0; i < userdisk_maxunit; i++) {
			if (CALLBACK(diskioctl, i, DIOCGSECTORSIZE,
			    &sectorsize) != 0 || CALLBACK(diskioctl, i,
			    DIOCGMEDIASIZE, &mediasize) != 0)
				return (ENXIO);
			ud_info[i].mediasize = mediasize;
			ud_info[i].sectorsize = sectorsize;
		}
	}

	return(0);
}

static void
userdisk_cleanup(void)
{

	if (userdisk_maxunit > 0)
		free(ud_info);
	disk_cleanup(&userboot_disk);
}

/*
 * Print information about disks
 */
static void
userdisk_print(int verbose)
{
	struct disk_devdesc dev;
	char line[80];
	int i;

	for (i = 0; i < userdisk_maxunit; i++) {
		sprintf(line, "    disk%d:   Guest drive image\n", i);
		pager_output(line);
		dev.d_dev = &userboot_disk;
		dev.d_unit = i;
		dev.d_slice = -1;
		dev.d_partition = -1;
		if (disk_open(&dev, ud_info[i].mediasize,
		    ud_info[i].sectorsize, 0) == 0) {
			sprintf(line, "    disk%d", i);
			disk_print(&dev, line, verbose);
			disk_close(&dev);
		}
	}
}

/*
 * Attempt to open the disk described by (dev) for use by (f).
 */
static int
userdisk_open(struct open_file *f, ...)
{
	va_list			ap;
	struct disk_devdesc	*dev;

	va_start(ap, f);
	dev = va_arg(ap, struct disk_devdesc *);
	va_end(ap);

	if (dev->d_unit < 0 || dev->d_unit >= userdisk_maxunit)
		return (EIO);

	return (disk_open(dev, ud_info[dev->d_unit].mediasize,
	    ud_info[dev->d_unit].sectorsize, 0));
}

static int
userdisk_close(struct open_file *f)
{
	struct disk_devdesc *dev;

	dev = (struct disk_devdesc *)f->f_devdata;
	return (disk_close(dev));
}

static int
userdisk_strategy(void *devdata, int rw, daddr_t dblk, size_t size,
    char *buf, size_t *rsize)
{
	struct disk_devdesc *dev = devdata;
	uint64_t	off;
	size_t		resid;
	int		rc;

	if (rw == F_WRITE)
		return (EROFS);
	if (rw != F_READ)
		return (EINVAL);
	if (rsize)
		*rsize = 0;
	off = (dblk + dev->d_offset) * ud_info[dev->d_unit].sectorsize;
	rc = CALLBACK(diskread, dev->d_unit, off, buf, size, &resid);
	if (rc)
		return (rc);
	if (rsize)
		*rsize = size - resid;
	return (0);
}

static int
userdisk_ioctl(struct open_file *f, u_long cmd, void *data)
{
	struct disk_devdesc *dev;

	dev = (struct disk_devdesc *)f->f_devdata;
	return (CALLBACK(diskioctl, dev->d_unit, cmd, data));
}