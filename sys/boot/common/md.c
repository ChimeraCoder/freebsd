
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
#include <sys/endian.h>
#include <sys/queue.h>
#include <machine/stdarg.h>
#include <stand.h>

#include "bootstrap.h"

#define	MD_BLOCK_SIZE	512

#ifndef MD_IMAGE_SIZE
#error Must be compiled with MD_IMAGE_SIZE defined
#endif
#if (MD_IMAGE_SIZE == 0 || MD_IMAGE_SIZE % MD_BLOCK_SIZE)
#error Image size must be a multiple of 512.
#endif

/*
 * Preloaded image gets put here.
 * Applications that patch the object with the image can determine
 * the size looking at the start and end markers (strings),
 * so we want them contiguous.
 */
static struct {
	u_char start[MD_IMAGE_SIZE];
	u_char end[128];
} md_image = {
	.start = "MFS Filesystem goes here",
	.end = "MFS Filesystem had better STOP here",
};

/* devsw I/F */
static int md_init(void);
static int md_strategy(void *, int, daddr_t, size_t, char *, size_t *);
static int md_open(struct open_file *, ...);
static int md_close(struct open_file *);
static void md_print(int);

struct devsw md_dev = {
	"md",
	DEVT_DISK,
	md_init,
	md_strategy,
	md_open,
	md_close,
	noioctl,
	md_print
};

static int
md_init(void)
{

	return (0);
}

static int
md_strategy(void *devdata, int rw, daddr_t blk, size_t size, char *buf,
    size_t *rsize)
{
	struct devdesc *dev = (struct devdesc *)devdata;
	size_t ofs;

	if (dev->d_unit != 0)
		return (ENXIO);

	if (blk < 0 || blk >= (MD_IMAGE_SIZE / MD_BLOCK_SIZE))
		return (EIO);

	if (size % MD_BLOCK_SIZE)
		return (EIO);

	ofs = blk * MD_BLOCK_SIZE;
	if ((ofs + size) > MD_IMAGE_SIZE)
		size = MD_IMAGE_SIZE - ofs;

	if (rsize != 0)
		*rsize = size;

	switch (rw) {
	case F_READ:
		bcopy(md_image.start + ofs, buf, size);
		return (0);
	case F_WRITE:
		bcopy(buf, md_image.start + ofs, size);
		return (0);
	}

	return (ENODEV);
}

static int
md_open(struct open_file *f, ...)
{
	va_list ap;
	struct devdesc *dev;

	va_start(ap, f);
	dev = va_arg(ap, struct devdesc *);
	va_end(ap);

	if (dev->d_unit != 0)
		return (ENXIO);

	return (0);
}

static int
md_close(struct open_file *f)
{
	struct devdesc *dev;

	dev = (struct devdesc *)(f->f_devdata);
	return ((dev->d_unit != 0) ? ENXIO : 0);
}

static void
md_print(int verbose)
{

	printf("MD (%u bytes)\n", MD_IMAGE_SIZE);
}