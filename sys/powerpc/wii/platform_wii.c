
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
#include <sys/pcpu.h>
#include <sys/proc.h>
#include <sys/reboot.h>
#include <sys/smp.h>
#include <sys/fbio.h>

#include <vm/vm.h>
#include <vm/pmap.h>

#include <machine/bus.h>
#include <machine/cpu.h>
#include <machine/hid.h>
#include <machine/platform.h>
#include <machine/platformvar.h>
#include <machine/pmap.h>
#include <machine/smp.h>
#include <machine/spr.h>
#include <machine/vmparam.h>

#include <powerpc/wii/wii_fbreg.h>
#include <powerpc/wii/wii_ipcreg.h>

#include "platform_if.h"

static int		wii_probe(platform_t);
static int		wii_attach(platform_t);
static void		wii_mem_regions(platform_t, struct mem_region **,
			    int *, struct mem_region **, int *);
static unsigned long	wii_timebase_freq(platform_t, struct cpuref *cpuref);
static void		wii_reset(platform_t);
static void		wii_cpu_idle(sbintime_t sbt);

static platform_method_t wii_methods[] = {
	PLATFORMMETHOD(platform_probe,		wii_probe),
	PLATFORMMETHOD(platform_attach,		wii_attach),
	PLATFORMMETHOD(platform_mem_regions,	wii_mem_regions),
	PLATFORMMETHOD(platform_timebase_freq,	wii_timebase_freq),
	PLATFORMMETHOD(platform_reset,		wii_reset),
 
	PLATFORMMETHOD_END
};

static platform_def_t wii_platform = {
	"wii",
	wii_methods,
	0
};

PLATFORM_DEF(wii_platform);

static int
wii_probe(platform_t plat)
{
	register_t vers = mfpvr();

	/*
	 * The Wii includes a PowerPC 750CL with custom modifications
	 * ("Broadway").
	 * For now, we just assume that if we are running on a
	 * PowerPC 750CL, then this platform is a Nintendo Wii.
	 */
	if ((vers & 0xfffff0e0) == (MPC750 << 16 | MPC750CL))
		return (BUS_PROBE_SPECIFIC);

	return (ENXIO);
}

static int
wii_attach(platform_t plat)
{
	cpu_idle_hook = wii_cpu_idle;

	return (0);
}

#define MEM_REGIONS     2
static struct mem_region avail_regions[MEM_REGIONS];

static void
wii_mem_regions(platform_t plat, struct mem_region **phys, int *physsz,
    struct mem_region **avail, int *availsz)
{
	/* 24MB 1T-SRAM */
	avail_regions[0].mr_start = 0x00000000;
	avail_regions[0].mr_size  = 0x01800000;

	/*
	 * Reserve space for the framebuffer which is located
	 * at the end of this 24MB memory region. See wii_fbreg.h.
	 */
	avail_regions[0].mr_size -= WIIFB_FB_LEN;

	/* 64MB GDDR3 SDRAM */
	avail_regions[1].mr_start = 0x10000000;
	avail_regions[1].mr_size  = 0x04000000;

	/*
	 * Reserve space for the DSP.
	 */
	avail_regions[1].mr_start += 0x4000;
	avail_regions[1].mr_size -= 0x4000;

	/*
	 * Reserve space for the IOS I/O memory.
	 */
	avail_regions[1].mr_size -= WIIIPC_IOH_LEN + 1;

	*phys = *avail = avail_regions;
	*physsz = *availsz = MEM_REGIONS;
}

static u_long
wii_timebase_freq(platform_t plat, struct cpuref *cpuref)
{
	
	/* Bus Frequency (243MHz) / 4 */
	return (60750000);
}

static void
wii_reset(platform_t plat)
{
}

static void
wii_cpu_idle(sbintime_t sbt)
{
}