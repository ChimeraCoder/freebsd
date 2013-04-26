
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

#include "opt_ddb.h"
#include "opt_platform.h"

#include <sys/param.h>
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/systm.h>
#include <sys/imgact.h>
#include <sys/bio.h>
#include <sys/buf.h>
#include <sys/bus.h>
#include <sys/cpu.h>
#include <sys/cons.h>
#include <sys/exec.h>
#include <sys/linker.h>
#include <sys/ucontext.h>
#include <sys/proc.h>
#include <sys/kdb.h>
#include <sys/ptrace.h>
#include <sys/reboot.h>
#include <sys/signalvar.h>
#include <sys/sysent.h>
#include <sys/sysproto.h>
#include <sys/user.h>

#ifdef FDT
#include <dev/fdt/fdt_common.h>
#include <dev/ofw/openfirm.h>
#endif

#include <vm/vm.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>

#include <machine/clock.h>
#include <machine/cpu.h>
#include <machine/cpuregs.h>
#include <machine/hwfunc.h>
#include <machine/md_var.h>
#include <machine/metadata.h>
#include <machine/pmap.h>
#include <machine/trap.h>

extern int	*edata;
extern int	*end;

void
platform_cpu_init()
{
	/* Nothing special */
}

static void
mips_init(void)
{
	int i;

	for (i = 0; i < 10; i++) {
		phys_avail[i] = 0;
	}

	/* phys_avail regions are in bytes */
	phys_avail[0] = MIPS_KSEG0_TO_PHYS(kernel_kseg0_end);
	phys_avail[1] = ctob(realmem);

	dump_avail[0] = phys_avail[0];
	dump_avail[1] = phys_avail[1];

	physmem = realmem;

	init_param1();
	init_param2(physmem);
	mips_cpu_init();
	pmap_bootstrap();
	mips_proc0_init();
	mutex_init();
	kdb_init();
#ifdef KDB
	if (boothowto & RB_KDB)
		kdb_enter(KDB_WHY_BOOTFLAGS, "Boot flags requested debugger");
#endif
}

/*
 * Perform a board-level soft-reset.
 *
 * XXXRW: BERI doesn't yet have a board-level soft-reset.
 */
void
platform_reset(void)
{

	panic("%s: not yet", __func__);
}

void
platform_start(__register_t a0, __register_t a1,  __register_t a2, 
    __register_t a3)
{
	vm_offset_t kernend;
	uint64_t platform_counter_freq;
	int argc = a0;
	char **argv = (char **)a1;
	char **envp = (char **)a2;
	unsigned int memsize = a3;
#ifdef FDT
	vm_offset_t dtbp;
	void *kmdp;
#endif
	int i;

	/* clear the BSS and SBSS segments */
	kernend = (vm_offset_t)&end;
	memset(&edata, 0, kernend - (vm_offset_t)(&edata));

	mips_postboot_fixup();

	mips_pcpu0_init();

#ifdef FDT
	/*
	 * Find the dtb passed in by the boot loader (currently fictional).
	 */
	kmdp = preload_search_by_type("elf kernel");
	if (kmdp != NULL)
		dtbp = MD_FETCH(kmdp, MODINFOMD_DTBP, vm_offset_t);
	else
		dtbp = (vm_offset_t)NULL;

#if defined(FDT_DTB_STATIC)
	/*
	 * In case the device tree blob was not retrieved (from metadata) try
	 * to use the statically embedded one.
	 */
	if (dtbp == (vm_offset_t)NULL)
		dtbp = (vm_offset_t)&fdt_static_dtb;
#else
#error	"Non-static FDT not yet supported on BERI"
#endif

	if (OF_install(OFW_FDT, 0) == FALSE)
		while (1);
	if (OF_init(&fdt_static_dtb) != 0)
		while (1);
#endif

	/*
	 * XXXRW: We have no way to compare wallclock time to cycle rate on
	 * BERI, so for now assume we run at the MALTA default (100MHz).
	 */
	platform_counter_freq = MIPS_DEFAULT_HZ;
	mips_timer_early_init(platform_counter_freq);

	cninit();
	printf("entry: platform_start()\n");

	bootverbose = 1;
	if (bootverbose) {
		printf("cmd line: ");
		for (i = 0; i < argc; i++)
			printf("%s ", argv[i]);
		printf("\n");

		printf("envp:\n");
		for (i = 0; envp[i]; i += 2)
			printf("\t%s = %s\n", envp[i], envp[i+1]);

		printf("memsize = %08x\n", memsize);
	}

	realmem = btoc(memsize);
	mips_init();

	mips_timer_init_params(platform_counter_freq, 0);
}