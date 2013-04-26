
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
#include <sys/kernel.h>
#include <sys/limits.h>
#include <sys/malloc.h>
#include <sys/queue.h>
#include <sys/sysctl.h>
#include <sys/systm.h>

#include <geom/geom.h>


#define	G_ZERO_CLASS_NAME	"ZERO"

SYSCTL_DECL(_kern_geom);
static SYSCTL_NODE(_kern_geom, OID_AUTO, zero, CTLFLAG_RW, 0,
    "GEOM_ZERO stuff");
static int g_zero_clear = 1;
SYSCTL_INT(_kern_geom_zero, OID_AUTO, clear, CTLFLAG_RW, &g_zero_clear, 0,
    "Clear read data buffer");
static int g_zero_byte = 0;
SYSCTL_INT(_kern_geom_zero, OID_AUTO, byte, CTLFLAG_RW, &g_zero_byte, 0,
    "Byte (octet) value to clear the buffers with");

static void
g_zero_start(struct bio *bp)
{
	int error = ENXIO;

	switch (bp->bio_cmd) {
	case BIO_READ:
		if (g_zero_clear)
			memset(bp->bio_data, g_zero_byte, bp->bio_length);
		/* FALLTHROUGH */
	case BIO_DELETE:
	case BIO_WRITE:
		bp->bio_completed = bp->bio_length;
		error = 0;
		break;
	case BIO_GETATTR:
	default:
		error = EOPNOTSUPP;
		break;
	}
	g_io_deliver(bp, error);
}

static void
g_zero_init(struct g_class *mp)
{
	struct g_geom *gp;
	struct g_provider *pp;

	g_topology_assert();
	gp = g_new_geomf(mp, "gzero");
	gp->start = g_zero_start;
	gp->access = g_std_access;
	pp = g_new_providerf(gp, "%s", gp->name);
	pp->mediasize = 1152921504606846976LLU;
	pp->sectorsize = 512;
	g_error_provider(pp, 0);
}

static int
g_zero_destroy_geom(struct gctl_req *req __unused, struct g_class *mp __unused,
    struct g_geom *gp)
{
	struct g_provider *pp;

	g_topology_assert();
	if (gp == NULL)
		return (0);
	pp = LIST_FIRST(&gp->provider);
	if (pp == NULL)
		return (0);
	if (pp->acr > 0 || pp->acw > 0 || pp->ace > 0)
		return (EBUSY);
	g_wither_geom(gp, ENXIO);
	return (0);
}

static struct g_class g_zero_class = {
	.name = G_ZERO_CLASS_NAME,
	.version = G_VERSION,
	.init = g_zero_init,
	.destroy_geom = g_zero_destroy_geom
};

DECLARE_GEOM_CLASS(g_zero_class, g_zero);