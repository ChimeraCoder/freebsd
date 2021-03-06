
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
#include <sys/bio.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/mutex.h>
#include <sys/rman.h>

#include <geom/geom.h>

#include <machine/cpuregs.h>

#include <dev/gxemul/disk/gxemul_diskreg.h>

struct gxemul_disk_softc {
	device_t sc_dev;
	uint64_t sc_size;
	struct g_geom *sc_geom;
	struct g_provider *sc_provider;
};

static struct mtx gxemul_disk_controller_mutex;

static g_start_t	gxemul_disk_start;
static g_access_t	gxemul_disk_access;

struct g_class g_gxemul_disk_class = {
	.name = "GXemul",
	.version = G_VERSION,
	.start = gxemul_disk_start,
	.access = gxemul_disk_access,
};

DECLARE_GEOM_CLASS(g_gxemul_disk_class, g_gxemul_disk);

static void	gxemul_disk_identify(driver_t *, device_t);
static int	gxemul_disk_probe(device_t);
static int	gxemul_disk_attach(device_t);
static void	gxemul_disk_attach_geom(void *, int);

static int	gxemul_disk_read(unsigned, void *, off_t);
static int	gxemul_disk_size(unsigned, uint64_t *);
static int	gxemul_disk_write(unsigned, const void *, off_t);

static void
gxemul_disk_start(struct bio *bp)
{
	struct gxemul_disk_softc *sc;
	unsigned diskid;
	off_t offset;
	uint8_t *buf;
	int error;

	sc = bp->bio_to->geom->softc;
	diskid = device_get_unit(sc->sc_dev);

	if ((bp->bio_length % GXEMUL_DISK_DEV_BLOCKSIZE) != 0) {
		g_io_deliver(bp, EINVAL);
		return;
	}

	buf = bp->bio_data;
	offset = bp->bio_offset;
	bp->bio_resid = bp->bio_length;
	while (bp->bio_resid != 0) {
		switch (bp->bio_cmd) {
		case BIO_READ:
			mtx_lock(&gxemul_disk_controller_mutex);
			error = gxemul_disk_read(diskid, buf, offset);
			mtx_unlock(&gxemul_disk_controller_mutex);
			break;
		case BIO_WRITE:
			mtx_lock(&gxemul_disk_controller_mutex);
			error = gxemul_disk_write(diskid, buf, offset);
			mtx_unlock(&gxemul_disk_controller_mutex);
			break;
		default:
			g_io_deliver(bp, EOPNOTSUPP);
			return;
		}
		if (error != 0) {
			g_io_deliver(bp, error);
			return;
		}

		buf += GXEMUL_DISK_DEV_BLOCKSIZE;
		offset += GXEMUL_DISK_DEV_BLOCKSIZE;
		bp->bio_completed += GXEMUL_DISK_DEV_BLOCKSIZE;
		bp->bio_resid -= GXEMUL_DISK_DEV_BLOCKSIZE;
	}

	g_io_deliver(bp, 0);
}

static int
gxemul_disk_access(struct g_provider *pp, int r, int w, int e)
{
	return (0);
}

static void
gxemul_disk_identify(driver_t *drv, device_t parent)
{
	unsigned diskid;

	mtx_init(&gxemul_disk_controller_mutex, "GXemul disk controller", NULL, MTX_DEF);

	mtx_lock(&gxemul_disk_controller_mutex);
	for (diskid = 0; diskid < 0x100; diskid++) {
		/*
		 * If we can read at offset 0, this disk id must be
		 * present enough.  If we get an error, stop looking.
		 * Disks in GXemul are allocated linearly from 0.
		 */
		if (gxemul_disk_read(diskid, NULL, 0) != 0)
			break;
		BUS_ADD_CHILD(parent, 0, "gxemul_disk", diskid);
	}
	mtx_unlock(&gxemul_disk_controller_mutex);
}

static int
gxemul_disk_probe(device_t dev)
{
	device_set_desc(dev, "GXemul test disk");

	return (0);
}

static void
gxemul_disk_attach_geom(void *arg, int flag)
{
	struct gxemul_disk_softc *sc;

	sc = arg;

	sc->sc_geom = g_new_geomf(&g_gxemul_disk_class, "%s", device_get_nameunit(sc->sc_dev));
	sc->sc_geom->softc = sc;

	sc->sc_provider = g_new_providerf(sc->sc_geom, sc->sc_geom->name);
	sc->sc_provider->sectorsize = GXEMUL_DISK_DEV_BLOCKSIZE;
	sc->sc_provider->mediasize = sc->sc_size;
	g_error_provider(sc->sc_provider, 0);
}

static int
gxemul_disk_attach(device_t dev)
{
	struct gxemul_disk_softc *sc;
	unsigned diskid;
	int error;

	diskid = device_get_unit(dev);

	sc = device_get_softc(dev);
	sc->sc_dev = dev;
	sc->sc_geom = NULL;
	sc->sc_provider = NULL;

	mtx_lock(&gxemul_disk_controller_mutex);
	error = gxemul_disk_size(diskid, &sc->sc_size);
	if (error != 0) {
		mtx_unlock(&gxemul_disk_controller_mutex);
		return (error);
	}
	mtx_unlock(&gxemul_disk_controller_mutex);

	g_post_event(gxemul_disk_attach_geom, sc, M_WAITOK, NULL);

	return (0);
}

static int
gxemul_disk_read(unsigned diskid, void *buf, off_t off)
{
	const volatile void *src;

	mtx_assert(&gxemul_disk_controller_mutex, MA_OWNED);

	if (off < 0 || off % GXEMUL_DISK_DEV_BLOCKSIZE != 0)
		return (EINVAL);

	GXEMUL_DISK_DEV_WRITE(GXEMUL_DISK_DEV_OFFSET, (uint64_t)off);
	GXEMUL_DISK_DEV_WRITE(GXEMUL_DISK_DEV_DISKID, diskid);
	GXEMUL_DISK_DEV_WRITE(GXEMUL_DISK_DEV_START, GXEMUL_DISK_DEV_START_READ);
	switch (GXEMUL_DISK_DEV_READ(GXEMUL_DISK_DEV_STATUS)) {
	case GXEMUL_DISK_DEV_STATUS_FAILURE:
		return (EIO);
	default:
		break;
	}

	if (buf != NULL) {
		src = GXEMUL_DISK_DEV_FUNCTION(GXEMUL_DISK_DEV_BLOCK);
		memcpy(buf, (const void *)(uintptr_t)src,
		       GXEMUL_DISK_DEV_BLOCKSIZE);
	}

	return (0);
}

static int
gxemul_disk_size(unsigned diskid, uint64_t *sizep)
{
	uint64_t offset, ogood;
	uint64_t m, s;
	int error;

	m = 1;
	s = 3;
	ogood = 0;

	for (;;) {
		offset = (ogood * s) + (m * GXEMUL_DISK_DEV_BLOCKSIZE);

		error = gxemul_disk_read(diskid, NULL, offset);
		if (error != 0) {
			if (m == 1 && s == 1) {
				*sizep = ogood + GXEMUL_DISK_DEV_BLOCKSIZE;
				return (0);
			}
			if (m > 1)
				m /= 2;
			if (s > 1)
				s--;
			continue;
		}
		if (ogood == offset) {
			m = 1;
			continue;
		}
		ogood = offset;
		m++;
	}

	return (EDOOFUS);
}

static int
gxemul_disk_write(unsigned diskid, const void *buf, off_t off)
{
	volatile void *dst;

	mtx_assert(&gxemul_disk_controller_mutex, MA_OWNED);

	if (off < 0 || off % GXEMUL_DISK_DEV_BLOCKSIZE != 0)
		return (EINVAL);

	GXEMUL_DISK_DEV_WRITE(GXEMUL_DISK_DEV_OFFSET, (uint64_t)off);
	GXEMUL_DISK_DEV_WRITE(GXEMUL_DISK_DEV_DISKID, diskid);

	dst = GXEMUL_DISK_DEV_FUNCTION(GXEMUL_DISK_DEV_BLOCK);
	memcpy((void *)(uintptr_t)dst, buf, GXEMUL_DISK_DEV_BLOCKSIZE);

	GXEMUL_DISK_DEV_WRITE(GXEMUL_DISK_DEV_START, GXEMUL_DISK_DEV_START_WRITE);
	switch (GXEMUL_DISK_DEV_READ(GXEMUL_DISK_DEV_STATUS)) {
	case GXEMUL_DISK_DEV_STATUS_FAILURE:
		return (EIO);
	default:
		break;
	}

	return (0);
}

static device_method_t gxemul_disk_methods[] = {
	DEVMETHOD(device_probe,		gxemul_disk_probe),
	DEVMETHOD(device_identify,      gxemul_disk_identify),
	DEVMETHOD(device_attach,	gxemul_disk_attach),

	{ 0, 0 }
};

static driver_t gxemul_disk_driver = {
	"gxemul_disk", 
	gxemul_disk_methods, 
	sizeof (struct gxemul_disk_softc)
};

static devclass_t gxemul_disk_devclass;
DRIVER_MODULE(gxemul_disk, nexus, gxemul_disk_driver, gxemul_disk_devclass, 0, 0);