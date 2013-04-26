
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

#pragma ident	"%Z%%M%	%I%	%E% SMI"

#include <sys/types.h>
#include <sys/bitmap.h>
#include <assert.h>
#include <strings.h>
#include <stdlib.h>

#include <dt_regset.h>

dt_regset_t *
dt_regset_create(ulong_t size)
{
	ulong_t n = BT_BITOUL(size + 1); /* + 1 for %r0 */
	dt_regset_t *drp = malloc(sizeof (dt_regset_t));

	if (drp == NULL)
		return (NULL);

	drp->dr_bitmap = malloc(sizeof (ulong_t) * n);
	drp->dr_size = size + 1;

	if (drp->dr_bitmap == NULL) {
		dt_regset_destroy(drp);
		return (NULL);
	}

	bzero(drp->dr_bitmap, sizeof (ulong_t) * n);
	return (drp);
}

void
dt_regset_destroy(dt_regset_t *drp)
{
	free(drp->dr_bitmap);
	free(drp);
}

void
dt_regset_reset(dt_regset_t *drp)
{
	bzero(drp->dr_bitmap, sizeof (ulong_t) * BT_BITOUL(drp->dr_size));
}

int
dt_regset_alloc(dt_regset_t *drp)
{
	ulong_t nbits = drp->dr_size - 1;
	ulong_t maxw = nbits >> BT_ULSHIFT;
	ulong_t wx;

	for (wx = 0; wx <= maxw; wx++) {
		if (drp->dr_bitmap[wx] != ~0UL)
			break;
	}

	if (wx <= maxw) {
		ulong_t maxb = (wx == maxw) ? nbits & BT_ULMASK : BT_NBIPUL - 1;
		ulong_t word = drp->dr_bitmap[wx];
		ulong_t bit, bx;
		int reg;

		for (bit = 1, bx = 0; bx <= maxb; bx++, bit <<= 1) {
			if ((word & bit) == 0) {
				reg = (int)((wx << BT_ULSHIFT) | bx);
				BT_SET(drp->dr_bitmap, reg);
				return (reg);
			}
		}
	}

	return (-1); /* no available registers */
}

void
dt_regset_free(dt_regset_t *drp, int reg)
{
	assert(reg > 0 && reg < drp->dr_size);
	assert(BT_TEST(drp->dr_bitmap, reg) != 0);
	BT_CLEAR(drp->dr_bitmap, reg);
}