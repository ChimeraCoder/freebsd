
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

#include <dev/fdt/fdt_common.h>
#include <dev/ofw/openfirm.h>

#include <machine/bus.h>
#include <machine/fdt.h>

/* FIXME move to header file */
#define TEGRA2_CLK_RST_PA_BASE		0x60006000

void
cpu_reset(void)
{
	bus_space_handle_t bsh;
	printf("Resetting...\n");
	bus_space_map(fdtbus_bs_tag,TEGRA2_CLK_RST_PA_BASE, 0x1000, 0, &bsh);
	bus_space_write_4(fdtbus_bs_tag, bsh, 4, 4);

	while(1);
}

struct fdt_fixup_entry fdt_fixup_table[] = {
	{ NULL, NULL }
};

static int
fdt_pic_decode_ic(phandle_t node, pcell_t *intr, int *interrupt, int *trig,
    int *pol)
{
	if (!fdt_is_compatible(node, "arm,gic"))
		return (ENXIO);

	*interrupt = fdt32_to_cpu(intr[0]);
	*trig = INTR_TRIGGER_CONFORM;
	*pol = INTR_POLARITY_CONFORM;
	return (0);
}

fdt_pic_decode_t fdt_pic_table[] = {
	&fdt_pic_decode_ic,
	NULL
};