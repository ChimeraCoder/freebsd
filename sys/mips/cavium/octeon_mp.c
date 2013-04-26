
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
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/smp.h>
#include <sys/systm.h>

#include <machine/hwfunc.h>
#include <machine/md_var.h>
#include <machine/smp.h>

#include <mips/cavium/octeon_pcmap_regs.h>

#include <contrib/octeon-sdk/cvmx.h>
#include <mips/cavium/octeon_irq.h>

unsigned octeon_ap_boot = ~0;

void
platform_ipi_send(int cpuid)
{
	cvmx_write_csr(CVMX_CIU_MBOX_SETX(cpuid), 1);
	mips_wbflush();
}

void
platform_ipi_clear(void)
{
	uint64_t action;

	action = cvmx_read_csr(CVMX_CIU_MBOX_CLRX(PCPU_GET(cpuid)));
	KASSERT(action == 1, ("unexpected IPIs: %#jx", (uintmax_t)action));
	cvmx_write_csr(CVMX_CIU_MBOX_CLRX(PCPU_GET(cpuid)), action);
}

int
platform_ipi_intrnum(void)
{
	return (1);
}

void
platform_init_ap(int cpuid)
{
	unsigned ciu_int_mask, clock_int_mask, ipi_int_mask;

	/*
	 * Set the exception base.
	 */
	mips_wr_ebase(0x80000000);

	/*
	 * Clear any pending IPIs.
	 */
	cvmx_write_csr(CVMX_CIU_MBOX_CLRX(cpuid), 0xffffffff);

	/*
	 * Set up interrupts.
	 */
	octeon_ciu_reset();

	/*
	 * Unmask the clock, ipi and ciu interrupts.
	 */
	ciu_int_mask = hard_int_mask(0);
	clock_int_mask = hard_int_mask(5);
	ipi_int_mask = hard_int_mask(platform_ipi_intrnum());
	set_intr_mask(ciu_int_mask | clock_int_mask | ipi_int_mask);

	mips_wbflush();
}

void
platform_cpu_mask(cpuset_t *mask)
{
	uint64_t core_mask = cvmx_sysinfo_get()->core_mask;
	uint64_t i, m;

	CPU_ZERO(mask);
	for (i = 0, m = 1 ; i < MAXCPU; i++, m <<= 1)
		if (core_mask & m)
			CPU_SET(i, mask);
}

struct cpu_group *
platform_smp_topo(void)
{
	return (smp_topo_none());
}

int
platform_start_ap(int cpuid)
{
	uint64_t cores_in_reset;

	/* 
	 * Release the core if it is in reset, and let it rev up a bit.
	 * The real synchronization happens below via octeon_ap_boot.
	 */
	cores_in_reset = cvmx_read_csr(CVMX_CIU_PP_RST);
	if (cores_in_reset & (1ULL << cpuid)) {
	    if (bootverbose)
		printf ("AP #%d still in reset\n", cpuid);
	    cores_in_reset &= ~(1ULL << cpuid);
	    cvmx_write_csr(CVMX_CIU_PP_RST, (uint64_t)(cores_in_reset));
	    DELAY(2000);    /* Give it a moment to start */
	}

	if (atomic_cmpset_32(&octeon_ap_boot, ~0, cpuid) == 0)
		return (-1);
	for (;;) {
		DELAY(1000);
		if (atomic_cmpset_32(&octeon_ap_boot, 0, ~0) != 0)
			return (0);
		printf("Waiting for cpu%d to start\n", cpuid);
	}
}