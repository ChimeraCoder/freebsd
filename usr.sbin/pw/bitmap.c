
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

#ifndef lint
static const char rcsid[] =
  "$FreeBSD$";
#endif /* not lint */

#include <stdlib.h>
#include <string.h>

#include "bitmap.h"

struct bitmap
bm_alloc(int size)
{
	struct bitmap   bm;
	int             szmap = (size / 8) + !!(size % 8);

	bm.size = size;
	bm.map = malloc(szmap);
	if (bm.map)
		memset(bm.map, 0, szmap);
	return bm;
}

void
bm_dealloc(struct bitmap * bm)
{
	free(bm->map);
}

static void
bm_getmask(int *pos, unsigned char *bmask)
{
	*bmask = (unsigned char) (1 << (*pos % 8));
	*pos /= 8;
}

void
bm_setbit(struct bitmap * bm, int pos)
{
	unsigned char   bmask;

	bm_getmask(&pos, &bmask);
	bm->map[pos] |= bmask;
}

void
bm_clrbit(struct bitmap * bm, int pos)
{
	unsigned char   bmask;

	bm_getmask(&pos, &bmask);
	bm->map[pos] &= ~bmask;
}

int
bm_isset(struct bitmap * bm, int pos)
{
	unsigned char   bmask;

	bm_getmask(&pos, &bmask);
	return !!(bm->map[pos] & bmask);
}

int
bm_firstunset(struct bitmap * bm)
{
	int             szmap = (bm->size / 8) + !!(bm->size % 8);
	int             at = 0;
	int             pos = 0;

	while (pos < szmap) {
		unsigned char   bmv = bm->map[pos++];
		unsigned char   bmask = 1;

		while (bmask & 0xff) {
			if ((bmv & bmask) == 0)
				return at;
			bmask <<= 1;
			++at;
		}
	}
	return at;
}

int
bm_lastset(struct bitmap * bm)
{
	int             szmap = (bm->size / 8) + !!(bm->size % 8);
	int             at = 0;
	int             pos = 0;
	int             ofs = 0;

	while (pos < szmap) {
		unsigned char   bmv = bm->map[pos++];
		unsigned char   bmask = 1;

		while (bmask & 0xff) {
			if ((bmv & bmask) != 0)
				ofs = at;
			bmask <<= 1;
			++at;
		}
	}
	return ofs;
}