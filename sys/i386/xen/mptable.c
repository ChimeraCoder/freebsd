
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
#include <sys/bus.h>
#include <sys/kernel.h>

#include <vm/vm.h>
#include <vm/vm_param.h>
#include <vm/pmap.h>

#include <machine/frame.h>
#include <machine/intr_machdep.h>
#include <machine/apicvar.h>

#include <xen/hypervisor.h>
#include <machine/xen/xen-os.h>
#include <machine/smp.h>
#include <xen/interface/vcpu.h>


static int	mptable_probe(void);
static int	mptable_probe_cpus(void);
static void	mptable_register(void *dummy);
static int	mptable_setup_local(void);
static int	mptable_setup_io(void);

static struct apic_enumerator mptable_enumerator = {
	"MPTable",
	mptable_probe,
	mptable_probe_cpus,
	mptable_setup_local,
	mptable_setup_io
};

static int
mptable_probe(void)
{

	return (-100);
}

static int
mptable_probe_cpus(void)
{
	int i, rc;

	for (i = 0; i < MAXCPU; i++) {
		rc = HYPERVISOR_vcpu_op(VCPUOP_is_up, i, NULL);
		if (rc >= 0)
			cpu_add(i, (i == 0));
	}

	return (0);
}

/*
 * Initialize the local APIC on the BSP.
 */
static int
mptable_setup_local(void)
{

	return (0);
}

static int
mptable_setup_io(void)
{

	return (0);
}

static void
mptable_register(void *dummy __unused)
{

	apic_register_enumerator(&mptable_enumerator);
}
SYSINIT(mptable_register, SI_SUB_TUNABLES - 1, SI_ORDER_FIRST, mptable_register,
    NULL);