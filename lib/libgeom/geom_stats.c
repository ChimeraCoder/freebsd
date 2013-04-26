
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

#include <paths.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgeom.h>

#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/devicestat.h>


/************************************************************/
static uint npages, pagesize, spp;
static int statsfd = -1;
static u_char *statp;

void
geom_stats_close(void)
{
	if (statsfd == -1)
		return;
	munmap(statp, npages *pagesize);
	statp = NULL;
	close (statsfd);
	statsfd = -1;
}

void
geom_stats_resync(void)
{
	void *p;

	if (statsfd == -1)
		return;
	for (;;) {
		p = mmap(statp, (npages + 1) * pagesize, 
		    PROT_READ, 0, statsfd, 0);
		if (p == MAP_FAILED)
			break;
		else
			statp = p;
		npages++;
	}
}

int
geom_stats_open(void)
{
	int error;
	void *p;

	if (statsfd != -1)
		return (EBUSY);
	statsfd = open(_PATH_DEV DEVSTAT_DEVICE_NAME, O_RDONLY);
	if (statsfd < 0)
		return (errno);
	pagesize = getpagesize();
	spp = pagesize / sizeof(struct devstat);
	p = mmap(NULL, pagesize, PROT_READ, 0, statsfd, 0);
	if (p == MAP_FAILED) {
		error = errno;
		close(statsfd);
		statsfd = -1;
		errno = error;
		return (error);
	}
	statp = p;
	npages = 1;
	geom_stats_resync();
	return (0);
}

struct snapshot {
	u_char		*ptr;
	uint		pages;
	uint		pagesize;
	uint		perpage;
	struct timespec	time;
	/* used by getnext: */
	uint		u, v;
};

void *
geom_stats_snapshot_get(void)
{
	struct snapshot *sp;

	sp = malloc(sizeof *sp);
	if (sp == NULL)
		return (NULL);
	memset(sp, 0, sizeof *sp);
	sp->ptr = malloc(pagesize * npages);
	if (sp->ptr == NULL) {
		free(sp);
		return (NULL);
	}
	memset(sp->ptr, 0, pagesize * npages); 	/* page in, cache */
	clock_gettime(CLOCK_REALTIME, &sp->time);
	memset(sp->ptr, 0, pagesize * npages); 	/* page in, cache */
	memcpy(sp->ptr, statp, pagesize * npages);
	sp->pages = npages;
	sp->perpage = spp;
	sp->pagesize = pagesize;
	return (sp);
}

void
geom_stats_snapshot_free(void *arg)
{
	struct snapshot *sp;

	sp = arg;
	free(sp->ptr);
	free(sp);
}

void
geom_stats_snapshot_timestamp(void *arg, struct timespec *tp)
{
	struct snapshot *sp;

	sp = arg;
	*tp = sp->time;
}

void
geom_stats_snapshot_reset(void *arg)
{
	struct snapshot *sp;

	sp = arg;
	sp->u = sp->v = 0;
}

struct devstat *
geom_stats_snapshot_next(void *arg)
{
	struct devstat *gsp;
	struct snapshot *sp;

	sp = arg;
	gsp = (struct devstat *)
	    (sp->ptr + sp->u * pagesize + sp->v * sizeof *gsp);
	if (++sp->v >= sp->perpage) {
		if (++sp->u >= sp->pages)
			return (NULL);
		else
			sp->v = 0;
	}
	return (gsp);
}