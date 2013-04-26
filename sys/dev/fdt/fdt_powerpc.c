
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
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/bus.h>

#include <machine/intr_machdep.h>

#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>
#include <dev/ofw/openfirm.h>

#include "ofw_bus_if.h"
#include "fdt_common.h"

static void
fdt_fixup_busfreq(phandle_t root)
{
	phandle_t sb, cpus, child;
	pcell_t freq;

	/*
	 * Do a strict check so as to skip non-SOC nodes, which also claim
	 * simple-bus compatibility such as eLBC etc.
	 */
	if ((sb = fdt_find_compatible(root, "simple-bus", 1)) == 0)
		return;

	/*
	 * This fixup uses /cpus/ bus-frequency prop value to set simple-bus
	 * bus-frequency property.
	 */
	if ((cpus = OF_finddevice("/cpus")) == -1)
		return;

	if ((child = OF_child(cpus)) == 0)
		return;

	if (OF_getprop(child, "bus-frequency", (void *)&freq,
	    sizeof(freq)) <= 0)
		return;

	OF_setprop(sb, "bus-frequency", (void *)&freq, sizeof(freq));
}

struct fdt_fixup_entry fdt_fixup_table[] = {
	{ "fsl,MPC8572DS", &fdt_fixup_busfreq },
	{ "MPC8555CDS", &fdt_fixup_busfreq },
	{ NULL, NULL }
};

static int
fdt_pic_decode_iic(phandle_t node, pcell_t *intr, int *interrupt, int *trig,
    int *pol)
{
	if (!fdt_is_compatible(node, "chrp,iic"))
		return (ENXIO);

	*interrupt = intr[0];

	switch (intr[1]) {
	case 0:
		/* Active L level */
		*trig = INTR_TRIGGER_LEVEL;
		*pol = INTR_POLARITY_LOW;
		break;
	case 1:
		/* Active H level */
		*trig = INTR_TRIGGER_LEVEL;
		*pol = INTR_POLARITY_HIGH;
		break;
	case 2:
		/* H to L edge */
		*trig = INTR_TRIGGER_EDGE;
		*pol = INTR_POLARITY_LOW;
		break;
	case 3:
		/* L to H edge */
		*trig = INTR_TRIGGER_EDGE;
		*pol = INTR_POLARITY_HIGH;
		break;
	default:
		*trig = INTR_TRIGGER_CONFORM;
		*pol = INTR_POLARITY_CONFORM;
	}
	return (0);
}

static int
fdt_pic_decode_openpic(phandle_t node, pcell_t *intr, int *interrupt,
    int *trig, int *pol)
{

	if (!fdt_is_compatible(node, "chrp,open-pic"))
		return (ENXIO);

	/*
	 * XXX The interrupt number read out from the MPC85XX device tree is
	 * already offset by 16 to reflect the 'internal' IRQ range shift on
	 * the OpenPIC.
	 */
	*interrupt = intr[0];

	switch (intr[1]) {
	case 0:
		/* L to H edge */
		*trig = INTR_TRIGGER_EDGE;
		*pol = INTR_POLARITY_HIGH;
		break;
	case 1:
		/* Active L level */
		*trig = INTR_TRIGGER_LEVEL;
		*pol = INTR_POLARITY_LOW;
		break;
	case 2:
		/* Active H level */
		*trig = INTR_TRIGGER_LEVEL;
		*pol = INTR_POLARITY_HIGH;
		break;
	case 3:
		/* H to L edge */
		*trig = INTR_TRIGGER_EDGE;
		*pol = INTR_POLARITY_LOW;
		break;
	default:
		*trig = INTR_TRIGGER_CONFORM;
		*pol = INTR_POLARITY_CONFORM;
	}
	return (0);
}

fdt_pic_decode_t fdt_pic_table[] = {
	&fdt_pic_decode_iic,
	&fdt_pic_decode_openpic,
	NULL
};