
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
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/smp.h>

#include <machine/smp.h>
#include <machine/fdt.h>
#include <machine/intr.h>

#include <arm/ti/ti_smc.h>
#include <arm/ti/omap4/omap4_smc.h>

void mpentry(void);
void mptramp(void);

void
platform_mp_init_secondary(void)
{
	gic_init_secondary();
}

void
platform_mp_setmaxid(void)
{

        mp_maxid = 1;
}

int
platform_mp_probe(void)
{

	mp_ncpus = 2;
	return (1);
}

void    
platform_mp_start_ap(void)
{
	bus_addr_t scu_addr;

	if (bus_space_map(fdtbus_bs_tag, 0x48240000, 0x1000, 0, &scu_addr) != 0)
		panic("Couldn't map the SCU\n");
	/* Enable the SCU */
	*(volatile unsigned int *)scu_addr |= 1;
	//*(volatile unsigned int *)(scu_addr + 0x30) |= 1;
	cpu_idcache_wbinv_all();
	cpu_l2cache_wbinv_all();
	ti_smc0(0x200, 0xfffffdff, MODIFY_AUX_CORE_0);
	ti_smc0(pmap_kextract((vm_offset_t)mpentry), 0, WRITE_AUX_CORE_1);
	armv7_sev();
	bus_space_unmap(fdtbus_bs_tag, scu_addr, 0x1000);
}

void
platform_ipi_send(cpuset_t cpus, u_int ipi)
{
	pic_ipi_send(cpus, ipi);
}