
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
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/systm.h>

#include <geom/geom.h>
#include <geom/vinum/geom_vinum_var.h>
#include <geom/vinum/geom_vinum.h>

void
gv_volume_flush(struct gv_volume *v)
{
	struct gv_softc *sc;
	struct bio *bp;

	KASSERT(v != NULL, ("NULL v"));
	sc = v->vinumconf;
	KASSERT(sc != NULL, ("NULL sc"));

	bp = bioq_takefirst(v->wqueue);
	while (bp != NULL) {
		gv_volume_start(sc, bp);
		bp = bioq_takefirst(v->wqueue);
	}
}

void
gv_volume_start(struct gv_softc *sc, struct bio *bp)
{
	struct g_geom *gp;
	struct gv_volume *v;
	struct gv_plex *p, *lp;
	int numwrites;

	gp = sc->geom;
	v = bp->bio_to->private;
	if (v == NULL || v->state != GV_VOL_UP) {
		g_io_deliver(bp, ENXIO);
		return;
	}

	switch (bp->bio_cmd) {
	case BIO_READ:
		/*
		 * Try to find a good plex where we can send the request to,
		 * round-robin-style.  The plex either has to be up, or it's a
		 * degraded RAID5 plex. Check if we have delayed requests. Put
		 * this request on the delayed queue if so. This makes sure that
		 * we don't read old values.
		 */
		if (bioq_first(v->wqueue) != NULL) {
			bioq_insert_tail(v->wqueue, bp);
			break;
		}
		lp = v->last_read_plex;
		if (lp == NULL)
			lp = LIST_FIRST(&v->plexes);
		p = LIST_NEXT(lp, in_volume);
		if (p == NULL)
			p = LIST_FIRST(&v->plexes);
		do {
			if (p == NULL) {
				p = lp;
				break;
			}
			if ((p->state > GV_PLEX_DEGRADED) ||
			    (p->state >= GV_PLEX_DEGRADED &&
			    p->org == GV_PLEX_RAID5))
				break;
			p = LIST_NEXT(p, in_volume);
			if (p == NULL)
				p = LIST_FIRST(&v->plexes);
		} while (p != lp);

		if ((p == NULL) ||
		    (p->org == GV_PLEX_RAID5 && p->state < GV_PLEX_DEGRADED) ||
		    (p->org != GV_PLEX_RAID5 && p->state <= GV_PLEX_DEGRADED)) {
			g_io_deliver(bp, ENXIO);
			return;
		}
		v->last_read_plex = p;

		/* Hand it down to the plex logic. */
		gv_plex_start(p, bp);
		break;

	case BIO_WRITE:
	case BIO_DELETE:
		/* Delay write-requests if any plex is synchronizing. */
		LIST_FOREACH(p, &v->plexes, in_volume) {
			if (p->flags & GV_PLEX_SYNCING) {
				bioq_insert_tail(v->wqueue, bp);
				return;
			}
		}

		numwrites = 0;
		/* Give the BIO to each plex of this volume. */
		LIST_FOREACH(p, &v->plexes, in_volume) {
			if (p->state < GV_PLEX_DEGRADED)
				continue;
			gv_plex_start(p, bp);
			numwrites++;
		}
		if (numwrites == 0)
			g_io_deliver(bp, ENXIO);
		break;
	}
}

void
gv_bio_done(struct gv_softc *sc, struct bio *bp)
{
	struct gv_volume *v;
	struct gv_plex *p;
	struct gv_sd *s;

	s = bp->bio_caller1;
	KASSERT(s != NULL, ("gv_bio_done: NULL s"));
	p = s->plex_sc;
	KASSERT(p != NULL, ("gv_bio_done: NULL p"));
	v = p->vol_sc;
	KASSERT(v != NULL, ("gv_bio_done: NULL v"));

	switch (p->org) {
	case GV_PLEX_CONCAT:
	case GV_PLEX_STRIPED:
		gv_plex_normal_done(p, bp);
		break;
	case GV_PLEX_RAID5:
		gv_plex_raid5_done(p, bp);
		break;
	}
}