
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
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <machine/bus.h>

bs_protos(generic);
bs_protos(generic_armv4);

static struct bus_space _base_tag = {
	/* cookie */
	NULL,

	/* mapping/unmapping */
	generic_bs_map,
	generic_bs_unmap,
	generic_bs_subregion,

	/* allocation/deallocation */
	generic_bs_alloc,
	generic_bs_free,

	/* barrier */
	generic_bs_barrier,

	/* read (single) */
	generic_bs_r_1,
	generic_armv4_bs_r_2,
	generic_bs_r_4,
	NULL,

	/* read (multiple) */
	generic_bs_rm_1,
	generic_armv4_bs_rm_2,
	generic_bs_rm_4,
	NULL,

	/* read region */
	generic_bs_rr_1,
	generic_armv4_bs_rr_2,
	generic_bs_rr_4,
	NULL,

	/* write (single) */
	generic_bs_w_1,
	generic_armv4_bs_w_2,
	generic_bs_w_4,
	NULL,

	/* write multiple */
	generic_bs_wm_1,
	generic_armv4_bs_wm_2,
	generic_bs_wm_4,
	NULL,

	/* write region */
	NULL,
	NULL,
	NULL,
	NULL,

	/* set multiple */
	NULL,
	NULL,
	NULL,
	NULL,

	/* set region */
	NULL,
	NULL,
	NULL,
	NULL,

	/* copy */
	NULL,
	NULL,
	NULL,
	NULL,

	/* read stream (single) */
	NULL,
	NULL,
	NULL,
	NULL,

	/* read multiple stream */
	NULL,
	generic_armv4_bs_rm_2,
	NULL,
	NULL,

	/* read region stream */
	NULL,
	NULL,
	NULL,
	NULL,

	/* write stream (single) */
	NULL,
	NULL,
	NULL,
	NULL,

	/* write multiple stream */
	NULL,
	generic_armv4_bs_wm_2,
	NULL,
	NULL,

	/* write region stream */
	NULL,
	NULL,
	NULL,
	NULL,
};

bus_space_tag_t fdtbus_bs_tag = &_base_tag;