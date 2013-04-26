
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

#include "opt_isa.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/cons.h>
#include <sys/kernel.h>

#ifdef DEV_ISA
#include <isa/isavar.h>
extern device_t isa_bus_device;
#endif

static device_t nexusdev;

static void	configure_first(void *);
static void	configure(void *);
static void	configure_final(void *);

SYSINIT(configure1, SI_SUB_CONFIGURE, SI_ORDER_FIRST, configure_first, NULL);
/* SI_ORDER_SECOND is hookable */
SYSINIT(configure2, SI_SUB_CONFIGURE, SI_ORDER_THIRD, configure, NULL);
/* SI_ORDER_MIDDLE is hookable */
SYSINIT(configure3, SI_SUB_CONFIGURE, SI_ORDER_ANY, configure_final, NULL);

/*
 * Determine i/o configuration for a machine.
 */
static void
configure_first(void *dummy)
{

	nexusdev = device_add_child(root_bus, "nexus", 0);
}

static void
configure(void *dummy)
{

	root_bus_configure();
#ifdef DEV_ISA
	/*
	 * We bypass isa_probe_children(9) here in order to avoid
	 * invasive probes done by identify-routines of ISA drivers,
	 * which in turn can trigger master/target aborts, and the
	 * addition of ISA hints, which might erroneously exist.
	 */
	if (isa_bus_device != NULL)
		(void)bus_generic_attach(isa_bus_device);
#endif
}

static void
configure_final(void *dummy)
{

	cninit_finish();
	cold = 0;
}