
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
#include <sys/systm.h>
#include <sys/bus.h>

#include <machine/bus.h>
#include <mips/atheros/ar71xx_bus_space_reversed.h>

static bs_r_1_proto(reversed);
static bs_r_2_proto(reversed);
static bs_w_1_proto(reversed);
static bs_w_2_proto(reversed);

/*
 * Bus space that handles offsets in word for 1/2 bytes read/write access.
 * Byte order of values is handled by device drivers itself. 
 */
static struct bus_space bus_space_reversed = {
	/* cookie */
	(void *) 0,

	/* mapping/unmapping */
	generic_bs_map,
	generic_bs_unmap,
	generic_bs_subregion,

	/* allocation/deallocation */
	NULL,
	NULL,

	/* barrier */
	generic_bs_barrier,

	/* read (single) */
	reversed_bs_r_1,
	reversed_bs_r_2,
	generic_bs_r_4,
	NULL,

	/* read multiple */
	generic_bs_rm_1,
	generic_bs_rm_2,
	generic_bs_rm_4,
	NULL,

	/* read region */
	generic_bs_rr_1,
	generic_bs_rr_2,
	generic_bs_rr_4,
	NULL,

	/* write (single) */
	reversed_bs_w_1,
	reversed_bs_w_2,
	generic_bs_w_4,
	NULL,

	/* write multiple */
	generic_bs_wm_1,
	generic_bs_wm_2,
	generic_bs_wm_4,
	NULL,

	/* write region */
	NULL,
	generic_bs_wr_2,
	generic_bs_wr_4,
	NULL,

	/* set multiple */
	NULL,
	NULL,
	NULL,
	NULL,

	/* set region */
	NULL,
	generic_bs_sr_2,
	generic_bs_sr_4,
	NULL,

	/* copy */
	NULL,
	generic_bs_c_2,
	NULL,
	NULL,

	/* read (single) stream */
	generic_bs_r_1,
	generic_bs_r_2,
	generic_bs_r_4,
	NULL,

	/* read multiple stream */
	generic_bs_rm_1,
	generic_bs_rm_2,
	generic_bs_rm_4,
	NULL,

	/* read region stream */
	generic_bs_rr_1,
	generic_bs_rr_2,
	generic_bs_rr_4,
	NULL,

	/* write (single) stream */
	generic_bs_w_1,
	generic_bs_w_2,
	generic_bs_w_4,
	NULL,

	/* write multiple stream */
	generic_bs_wm_1,
	generic_bs_wm_2,
	generic_bs_wm_4,
	NULL,

	/* write region stream */
	NULL,
	generic_bs_wr_2,
	generic_bs_wr_4,
	NULL,
};

bus_space_tag_t ar71xx_bus_space_reversed = &bus_space_reversed;

static uint8_t
reversed_bs_r_1(void *t, bus_space_handle_t h, bus_size_t o)
{

	return readb(h + (o &~ 3) + (3 - (o & 3)));
}

static void
reversed_bs_w_1(void *t, bus_space_handle_t h, bus_size_t o, u_int8_t v)
{

	writeb(h + (o &~ 3) + (3 - (o & 3)), v);
}

static uint16_t
reversed_bs_r_2(void *t, bus_space_handle_t h, bus_size_t o)
{

	return readw(h + (o &~ 3) + (2 - (o & 3)));
}

static void
reversed_bs_w_2(void *t, bus_space_handle_t h, bus_size_t o, uint16_t v)
{

	writew(h + (o &~ 3) + (2 - (o & 3)), v);
}