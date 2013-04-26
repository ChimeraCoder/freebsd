
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

#include "opt_isa.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/conf.h>
#include <sys/reboot.h>
#include <sys/kernel.h>
#include <sys/mount.h>
#include <sys/sysctl.h>
#include <sys/bus.h>
#include <sys/cons.h>

#include <machine/intr.h>
#include <machine/md_var.h>

static void	configure_first(void *);
static void	configure(void *);
static void	configure_final(void *);

SYSINIT(configure1, SI_SUB_CONFIGURE, SI_ORDER_FIRST, configure_first, NULL);
/* SI_ORDER_SECOND is hookable */
SYSINIT(configure2, SI_SUB_CONFIGURE, SI_ORDER_THIRD, configure, NULL);
/* SI_ORDER_MIDDLE is hookable */
SYSINIT(configure3, SI_SUB_CONFIGURE, SI_ORDER_ANY, configure_final, NULL);

#ifdef DEV_ISA
#include <isa/isavar.h>
device_t isa_bus_device = 0;
#endif

/*
 * Determine i/o configuration for a machine.
 */
static void
configure_first(void *dummy)
{

	device_add_child(root_bus, "nexus", 0);
}

static void
configure(void *dummy)
{

	root_bus_configure();

	/*
	 * Probe ISA devices after everything.
	 */
#ifdef DEV_ISA
	if (isa_bus_device)
		isa_probe_children(isa_bus_device);
#endif
}

static void
configure_final(void *dummy)
{

	cninit_finish();

	ia64_enable_intr();

	cold = 0;
}