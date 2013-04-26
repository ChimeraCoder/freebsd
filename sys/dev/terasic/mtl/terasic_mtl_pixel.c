
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
#include <sys/bus.h>
#include <sys/conf.h>
#include <sys/consio.h>				/* struct vt_mode */
#include <sys/fbio.h>				/* video_adapter_t */
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/rman.h>
#include <sys/uio.h>

#include <machine/bus.h>
#include <machine/resource.h>
#include <machine/vm.h>

#include <dev/terasic/mtl/terasic_mtl.h>

static d_mmap_t terasic_mtl_pixel_mmap;
static d_read_t terasic_mtl_pixel_read;
static d_write_t terasic_mtl_pixel_write;

static struct cdevsw mtl_pixel_cdevsw = {
	.d_version =	D_VERSION,
	.d_mmap =	terasic_mtl_pixel_mmap,
	.d_read =	terasic_mtl_pixel_read,
	.d_write =	terasic_mtl_pixel_write,
	.d_name =	"terasic_mtl_pixel",
};

/*
 * All I/O to/from the MTL pixel device must be 32-bit, and aligned to 32-bit.
 */
static int
terasic_mtl_pixel_read(struct cdev *dev, struct uio *uio, int flag)
{
	struct terasic_mtl_softc *sc;
	u_long offset, size;
	uint32_t v;
	int error;

	if (uio->uio_offset < 0 || uio->uio_offset % 4 != 0 ||
	    uio->uio_resid % 4 != 0)
		return (ENODEV);
	sc = dev->si_drv1;
	size = rman_get_size(sc->mtl_pixel_res);
	error = 0;
	if ((uio->uio_offset + uio->uio_resid < 0) ||
	    (uio->uio_offset + uio->uio_resid > size))
		return (ENODEV);
	while (uio->uio_resid > 0) {
		offset = uio->uio_offset;
		if (offset + sizeof(v) > size)
			return (ENODEV);
		v = bus_read_4(sc->mtl_pixel_res, offset);
		error = uiomove(&v, sizeof(v), uio);
		if (error)
			return (error);
	}
	return (error);
}

static int
terasic_mtl_pixel_write(struct cdev *dev, struct uio *uio, int flag)
{
	struct terasic_mtl_softc *sc;
	u_long offset, size;
	uint32_t v;
	int error;

	if (uio->uio_offset < 0 || uio->uio_offset % 4 != 0 ||
	    uio->uio_resid % 4 != 0)
		return (ENODEV);
	sc = dev->si_drv1;
	size = rman_get_size(sc->mtl_pixel_res);
	error = 0;
	while (uio->uio_resid > 0) {
		offset = uio->uio_offset;
		if (offset + sizeof(v) > size)
			return (ENODEV);
		error = uiomove(&v, sizeof(v), uio);
		if (error)
			return (error);
		bus_write_4(sc->mtl_pixel_res, offset, v);
	}
	return (error);
}

static int
terasic_mtl_pixel_mmap(struct cdev *dev, vm_ooffset_t offset,
    vm_paddr_t *paddr, int nprot, vm_memattr_t *memattr)
{
	struct terasic_mtl_softc *sc;
	int error;

	sc = dev->si_drv1;
	error = 0;
	if (trunc_page(offset) == offset &&
	    rman_get_size(sc->mtl_pixel_res) >= offset + PAGE_SIZE) {
		*paddr = rman_get_start(sc->mtl_pixel_res) + offset;
		*memattr = VM_MEMATTR_UNCACHEABLE;
	} else
		error = ENODEV;
	return (error);
}

int
terasic_mtl_pixel_attach(struct terasic_mtl_softc *sc)
{

	sc->mtl_pixel_cdev = make_dev(&mtl_pixel_cdevsw, sc->mtl_unit,
	    UID_ROOT, GID_WHEEL, 0400, "mtl_pixel%d", sc->mtl_unit);
	if (sc->mtl_pixel_cdev == NULL) {
		device_printf(sc->mtl_dev, "%s: make_dev failed\n", __func__);
		return (ENXIO);
	}
	/* XXXRW: Slight race between make_dev(9) and here. */
	sc->mtl_pixel_cdev->si_drv1 = sc;
	return (0);
}

void
terasic_mtl_pixel_detach(struct terasic_mtl_softc *sc)
{

	if (sc->mtl_pixel_cdev != NULL)
		destroy_dev(sc->mtl_pixel_cdev);
}