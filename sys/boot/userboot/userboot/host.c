
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
 * Read from the host filesystem
 */

#include <sys/param.h>
#include <sys/time.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <stand.h>
#include <bootstrap.h>

#include "libuserboot.h"

/*
 * Open a file.
 */
static int
host_open(const char *upath, struct open_file *f)
{

	if (f->f_dev != &host_dev)
		return (EINVAL);

	return (CALLBACK(open, upath, &f->f_fsdata));
}

static int
host_close(struct open_file *f)
{

        CALLBACK(close, f->f_fsdata);
	f->f_fsdata = (void *)0;

	return (0);
}

/*
 * Copy a portion of a file into memory.
 */
static int
host_read(struct open_file *f, void *start, size_t size, size_t *resid)
{

	return (CALLBACK(read, f->f_fsdata, start, size, resid));
}

/*
 * Don't be silly - the bootstrap has no business writing anything.
 */
static int
host_write(struct open_file *f, void *start, size_t size, size_t *resid)
{

	return (EROFS);
}

static off_t
host_seek(struct open_file *f, off_t offset, int where)
{

	return (CALLBACK(seek, f->f_fsdata, offset, where));
}

static int
host_stat(struct open_file *f, struct stat *sb)
{
	int mode;
	int uid;
	int gid;
	uint64_t size;

	CALLBACK(stat, f->f_fsdata, &mode, &uid, &gid, &size);
	sb->st_mode = mode;
	sb->st_uid = uid;
	sb->st_gid = gid;
	sb->st_size = size;
	return (0);
}

static int
host_readdir(struct open_file *f, struct dirent *d)
{
	uint32_t fileno;
	uint8_t type;
	size_t namelen;
	int rc;

	rc = CALLBACK(readdir, f->f_fsdata, &fileno, &type, &namelen,
            d->d_name);
	if (rc)
		return (rc);

	d->d_fileno = fileno;
	d->d_type = type;
	d->d_namlen = namelen;

	return (0);
}

static int
host_dev_init(void)
{

	return (0);
}

static void
host_dev_print(int verbose)
{
	char line[80];

	sprintf(line, "    host%d:   Host filesystem\n", 0);
	pager_output(line);
}

/*
 * 'Open' the host device.
 */
static int
host_dev_open(struct open_file *f, ...)
{
	va_list		args;
	struct devdesc	*dev;

	va_start(args, f);
	dev = va_arg(args, struct devdesc*);
	va_end(args);

	return (0);
}

static int
host_dev_close(struct open_file *f)
{

	return (0);
}

static int
host_dev_strategy(void *devdata, int rw, daddr_t dblk, size_t size,
    char *buf, size_t *rsize)
{

	return (ENOSYS);
}

struct fs_ops host_fsops = {
	"host",
	host_open,
	host_close,
	host_read,
	host_write,
	host_seek,
	host_stat,
	host_readdir
};

struct devsw host_dev = {
	.dv_name = "host",
	.dv_type = DEVT_NET,
	.dv_init = host_dev_init,
	.dv_strategy = host_dev_strategy,
	.dv_open = host_dev_open,
	.dv_close = host_dev_close,
	.dv_ioctl = noioctl,
	.dv_print = host_dev_print,
	.dv_cleanup = NULL
};