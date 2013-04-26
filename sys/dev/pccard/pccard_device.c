
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
#include <sys/malloc.h>
#include <sys/systm.h>
#include <sys/uio.h>

#include <sys/bus.h>
#include <machine/bus.h>
#include <sys/rman.h>
#include <machine/resource.h>

#include <dev/pccard/pccardreg.h>
#include <dev/pccard/pccardvar.h>
#include <dev/pccard/pccardvarp.h>
#include <dev/pccard/pccard_cis.h>

static	d_open_t	pccard_open;
static	d_close_t	pccard_close;
static	d_read_t	pccard_read;
static	d_ioctl_t	pccard_ioctl;

static struct cdevsw pccard_cdevsw = {
	.d_version =	D_VERSION,
	.d_open =	pccard_open,
	.d_close =	pccard_close,
	.d_read =	pccard_read,
	.d_ioctl =	pccard_ioctl,
	.d_name =	"pccard"
};

int
pccard_device_create(struct pccard_softc *sc)
{
	uint32_t minor;

	minor = device_get_unit(sc->dev) << 16;
	sc->cisdev = make_dev(&pccard_cdevsw, minor, 0, 0, 0666,
	    "pccard%u.cis", device_get_unit(sc->dev));
	sc->cisdev->si_drv1 = sc;
	return (0);
}

int
pccard_device_destroy(struct pccard_softc *sc)
{
	if (sc->cisdev)
		destroy_dev(sc->cisdev);
	return (0);
}

static int
pccard_build_cis(const struct pccard_tuple *tuple, void *argp)
{
	struct cis_buffer *cis;
	int i;
	uint8_t ch;

	cis = (struct cis_buffer *)argp;
	/*
	 * CISTPL_END is a special case, it has no length field.
	 */
	if (tuple->code == CISTPL_END) {
		if (cis->len + 1 > sizeof(cis->buffer))
			return (ENOSPC);
		cis->buffer[cis->len++] = tuple->code;
		return (0);
	}
	if (cis->len + 2 + tuple->length > sizeof(cis->buffer))
		return (ENOSPC);
	cis->buffer[cis->len++] = tuple->code;
	cis->buffer[cis->len++] = tuple->length;
	for (i = 0; i < tuple->length; i++) {
		ch = pccard_tuple_read_1(tuple, i);
		cis->buffer[cis->len++] = ch;
	}
	return (0);
}

static	int
pccard_open(struct cdev *dev, int oflags, int devtype, struct thread *td)
{
	device_t parent, child;
	device_t *kids;
	int cnt, err;
	struct pccard_softc *sc;

	sc = dev->si_drv1;
	if (sc->cis_open)
		return (EBUSY);
	parent = sc->dev;
	err = device_get_children(parent, &kids, &cnt);
	if (err)
		return err;
	if (cnt == 0) {
		free(kids, M_TEMP);
		sc->cis_open++;
		sc->cis = NULL;
		return (0);
	}
	child = kids[0];
	free(kids, M_TEMP);
	sc->cis = malloc(sizeof(*sc->cis), M_TEMP, M_ZERO | M_WAITOK);
	err = pccard_scan_cis(parent, child, pccard_build_cis, sc->cis);
	if (err) {
		free(sc->cis, M_TEMP);
		sc->cis = NULL;
		return (err);
	}
	sc->cis_open++;
	return (0);
}

static	int
pccard_close(struct cdev *dev, int fflags, int devtype, struct thread *td)
{
	struct pccard_softc *sc;

	sc = dev->si_drv1;
	free(sc->cis, M_TEMP);
	sc->cis = NULL;
	sc->cis_open = 0;
	return (0);
}

static	int
pccard_ioctl(struct cdev *dev, u_long cmd, caddr_t data, int fflag,
    struct thread *td)
{
	return (ENOTTY);
}

static	int
pccard_read(struct cdev *dev, struct uio *uio, int ioflag)
{
	struct pccard_softc *sc;

	sc = dev->si_drv1;
	/* EOF */
	if (sc->cis == NULL || uio->uio_offset > sc->cis->len)
		return (0);
	return (uiomove(sc->cis->buffer + uio->uio_offset,
	  MIN(uio->uio_resid, sc->cis->len - uio->uio_offset), uio));
}