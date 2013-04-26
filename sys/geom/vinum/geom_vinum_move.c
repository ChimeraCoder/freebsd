
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

#include <sys/libkern.h>
#include <sys/malloc.h>

#include <geom/geom.h>
#include <geom/vinum/geom_vinum_var.h>
#include <geom/vinum/geom_vinum.h>

void
gv_move(struct g_geom *gp, struct gctl_req *req)
{
	struct gv_softc *sc;
	struct gv_sd *s;
	struct gv_drive *d;
	char buf[20], *destination, *object;
	int *argc, *flags, i, type;

	sc = gp->softc;

	argc = gctl_get_paraml(req, "argc", sizeof(*argc));
	if (argc == NULL) {
		gctl_error(req, "no arguments given");
		return;
	}
	flags = gctl_get_paraml(req, "flags", sizeof(*flags));
	if (flags == NULL) {
		gctl_error(req, "no flags given");
		return;
	}
	destination = gctl_get_param(req, "destination", NULL);
	if (destination == NULL) {
		gctl_error(req, "no destination given");
		return;
	}
	if (gv_object_type(sc, destination) != GV_TYPE_DRIVE) {
		gctl_error(req, "destination '%s' is not a drive", destination);
		return;
	}
	d = gv_find_drive(sc, destination);

	/*
	 * We start with 1 here, because argv[0] on the command line is the
	 * destination drive.
	 */
	for (i = 1; i < *argc; i++) {
		snprintf(buf, sizeof(buf), "argv%d", i);
		object = gctl_get_param(req, buf, NULL);
		if (object == NULL)
			continue;

		type = gv_object_type(sc, object);
		if (type != GV_TYPE_SD) {
			gctl_error(req, "you can only move subdisks; "
			    "'%s' is not a subdisk", object);
			return;
		}

		s = gv_find_sd(sc, object);
		if (s == NULL) {
			gctl_error(req, "unknown subdisk '%s'", object);
			return;
		}
		gv_post_event(sc, GV_EVENT_MOVE_SD, s, d, *flags, 0);
	}
}

/* Move a subdisk. */
int
gv_move_sd(struct gv_softc *sc, struct gv_sd *cursd, 
    struct gv_drive *destination, int flags)
{
	struct gv_drive *d;
	struct gv_sd *newsd, *s, *s2;
	struct gv_plex *p;
	int err;

	g_topology_assert();
	KASSERT(cursd != NULL, ("gv_move_sd: NULL cursd"));
	KASSERT(destination != NULL, ("gv_move_sd: NULL destination"));

	d = cursd->drive_sc;

	if ((gv_consumer_is_open(d->consumer) ||
	    gv_consumer_is_open(destination->consumer)) &&
	    !(flags & GV_FLAG_F)) {
		G_VINUM_DEBUG(0, "consumers on current and destination drive "
		    " still open");
		return (GV_ERR_ISBUSY);
	}

	if (!(flags & GV_FLAG_F)) {
		G_VINUM_DEBUG(1, "-f flag not passed; move would be "
		    "destructive");
		return (GV_ERR_INVFLAG);
	}

	if (destination == cursd->drive_sc) {
		G_VINUM_DEBUG(1, "subdisk '%s' already on drive '%s'",
		    cursd->name, destination->name);
		return (GV_ERR_ISATTACHED);
	}

	/* XXX: Does it have to be part of a plex? */
	p = gv_find_plex(sc, cursd->plex);
	if (p == NULL) {
		G_VINUM_DEBUG(0, "subdisk '%s' is not part of a plex",
		    cursd->name);
		return (GV_ERR_NOTFOUND);
	}

	/* Stale the old subdisk. */
	err = gv_set_sd_state(cursd, GV_SD_STALE,
	    GV_SETSTATE_FORCE | GV_SETSTATE_CONFIG);
	if (err) {
		G_VINUM_DEBUG(0, "unable to set the subdisk '%s' to state "
		    "'stale'", cursd->name);
		return (err);
	}

	/*
	 * Create new subdisk. Ideally, we'd use gv_new_sd, but that requires
	 * us to create a string for it to parse, which is silly.
	 * TODO: maybe refactor gv_new_sd such that this is no longer the case.
	 */
	newsd = g_malloc(sizeof(struct gv_sd), M_WAITOK | M_ZERO);
	newsd->plex_offset = cursd->plex_offset;
	newsd->size = cursd->size;
	newsd->drive_offset = -1;
	strlcpy(newsd->name, cursd->name, sizeof(newsd->name));
	strlcpy(newsd->drive, destination->name, sizeof(newsd->drive));
	strlcpy(newsd->plex, cursd->plex, sizeof(newsd->plex));
	newsd->state = GV_SD_STALE;
	newsd->vinumconf = cursd->vinumconf;

	err = gv_sd_to_drive(newsd, destination);
	if (err) {
		/* XXX not enough free space? */
		g_free(newsd);
		return (err);
	}

	/* Replace the old sd by the new one. */
	LIST_FOREACH_SAFE(s, &p->subdisks, in_plex, s2) {
		if (s == cursd) {
			gv_rm_sd(sc, s);
		}
	}
	gv_sd_to_plex(newsd, p);
	LIST_INSERT_HEAD(&sc->subdisks, newsd, sd);
	/* Update volume size of plex. */
	if (p->vol_sc != NULL)
		gv_update_vol_size(p->vol_sc, gv_vol_size(p->vol_sc));
	gv_save_config(p->vinumconf);
	return (0);
}