
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
#include <sys/bus.h>
#include <sys/module.h>
#include <isa/isavar.h>
#include <isa/isa_common.h>
#include <machine/resource.h>

void
isa_hinted_child(device_t parent, const char *name, int unit)
{
	device_t	child;
	int		sensitive, start, count;
	int		order;

	/* device-specific flag overrides any wildcard */
	sensitive = 0;
	if (resource_int_value(name, unit, "sensitive", &sensitive) != 0)
		resource_int_value(name, -1, "sensitive", &sensitive);

	if (sensitive)
		order = ISA_ORDER_SENSITIVE;
	else
		order = ISA_ORDER_SPECULATIVE;

	child = BUS_ADD_CHILD(parent, order, name, unit);
	if (child == 0)
		return;

	start = 0;
	count = 0;
	resource_int_value(name, unit, "port", &start);
	resource_int_value(name, unit, "portsize", &count);
	if (start > 0 || count > 0)
		bus_set_resource(child, SYS_RES_IOPORT, 0, start, count);

	start = 0;
	count = 0;
	resource_int_value(name, unit, "maddr", &start);
	resource_int_value(name, unit, "msize", &count);
	if (start > 0 || count > 0)
		bus_set_resource(child, SYS_RES_MEMORY, 0, start, count);

	if (resource_int_value(name, unit, "irq", &start) == 0 && start > 0)
		bus_set_resource(child, SYS_RES_IRQ, 0, start, 1);

	if (resource_int_value(name, unit, "drq", &start) == 0 && start >= 0)
		bus_set_resource(child, SYS_RES_DRQ, 0, start, 1);

	if (resource_disabled(name, unit))
		device_disable(child);

	isa_set_configattr(child, (isa_get_configattr(child)|ISACFGATTR_HINTS));
}

static int
isa_match_resource_hint(device_t dev, int type, long value)
{
	struct isa_device* idev = DEVTOISA(dev);
	struct resource_list *rl = &idev->id_resources;
	struct resource_list_entry *rle;

	STAILQ_FOREACH(rle, rl, link) {
		if (rle->type != type)
			continue;
		if (rle->start <= value && rle->end >= value)
			return (1);
	}
	return (0);
}

void
isa_hint_device_unit(device_t bus, device_t child, const char *name, int *unitp)
{
	const char *s;
	long value;
	int line, matches, unit;

	line = 0;
	for (;;) {
		if (resource_find_dev(&line, name, &unit, "at", NULL) != 0)
			break;

		/* Must have an "at" for isa. */
		resource_string_value(name, unit, "at", &s);
		if (!(strcmp(s, device_get_nameunit(bus)) == 0 ||
		    strcmp(s, device_get_name(bus)) == 0))
			continue;

		/*
		 * Check for matching resources.  We must have at
		 * least one match.  Since I/O and memory resources
		 * cannot be shared, if we get a match on either of
		 * those, ignore any mismatches in IRQs or DRQs.
		 *
		 * XXX: We may want to revisit this to be more lenient
		 * and wire as long as it gets one match.
		 */
		matches = 0;
		if (resource_long_value(name, unit, "port", &value) == 0) {
			/*
			 * Floppy drive controllers are notorious for
			 * having a wide variety of resources not all
			 * of which include the first port that is
			 * specified by the hint (typically 0x3f0)
			 * (see the comment above
			 * fdc_isa_alloc_resources() in fdc_isa.c).
			 * However, they do all seem to include port +
			 * 2 (e.g. 0x3f2) so for a floppy device, look
			 * for 'value + 2' in the port resources
			 * instead of the hint value.
			 */
			if (strcmp(name, "fdc") == 0)
				value += 2;
			if (isa_match_resource_hint(child, SYS_RES_IOPORT,
			    value))
				matches++;
			else
				continue;
		}
		if (resource_long_value(name, unit, "maddr", &value) == 0) {
			if (isa_match_resource_hint(child, SYS_RES_MEMORY,
			    value))
				matches++;
			else
				continue;
		}
		if (matches > 0)
			goto matched;
		if (resource_long_value(name, unit, "irq", &value) == 0) {
			if (isa_match_resource_hint(child, SYS_RES_IRQ, value))
				matches++;
			else
				continue;
		}
		if (resource_long_value(name, unit, "drq", &value) == 0) {
			if (isa_match_resource_hint(child, SYS_RES_DRQ, value))
				matches++;
			else
				continue;
		}

	matched:
		if (matches > 0) {
			/* We have a winner! */
			*unitp = unit;
			break;
		}
	}
}