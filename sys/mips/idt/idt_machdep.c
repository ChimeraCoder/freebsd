
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
#include <sys/ucontext.h>
#include <sys/proc.h>
#include <sys/kdb.h>
#include <sys/ptrace.h>
#include <sys/reboot.h>
#include <sys/signalvar.h>
#include <sys/sysent.h>
#include <sys/sysproto.h>
#include <sys/user.h>

#include <vm/vm.h>
#include <vm/vm_object.h>
#include <vm/vm_page.h>

#include <machine/cache.h>
#include <machine/clock.h>
#include <machine/cpu.h>
#include <machine/cpuinfo.h>
#include <machine/cpufunc.h>
#include <machine/cpuregs.h>
#include <machine/hwfunc.h>
#include <machine/intr_machdep.h>
#include <machine/locore.h>
#include <machine/md_var.h>
#include <machine/pte.h>
#include <machine/sigframe.h>
#include <machine/trap.h>
#include <machine/vmparam.h>

extern int	*edata;
extern int	*end;

void
platform_cpu_init()
{
	/* Nothing special */
}

void
platform_reset(void)
{
	volatile unsigned int * p = (void *)0xb8008000;
	/* 
	 * TODO: we should take care of TLB stuff here. Otherwise
	 * board does not boots properly next time
	 */

	/* Write 0x8000_0001 to the Reset register */
	*p = 0x80000001;

	__asm __volatile("li	$25, 0xbfc00000");
	__asm __volatile("j	$25");
}

void
platform_start(__register_t a0, __register_t a1,
    __register_t a2 __unused, __register_t a3 __unused)
{
	uint64_t platform_counter_freq;
	vm_offset_t kernend;
	int argc = a0;
	char **argv = (char **)a1;
	int i, mem;


	/* clear the BSS and SBSS segments */
	kernend = (vm_offset_t)&end;
	memset(&edata, 0, kernend - (vm_offset_t)(&edata));

	mips_postboot_fixup();

	/* Initialize pcpu stuff */
	mips_pcpu0_init();

	/*
	 * Looking for mem=XXM argument
	 */
	mem = 0; /* Just something to start with */
	for (i=0; i < argc; i++) {
		if (strncmp(argv[i], "mem=", 4) == 0) {
			mem = strtol(argv[i] + 4, NULL, 0);
			break;
		}
	}

	bootverbose = 1;
	if (mem > 0)
		realmem = btoc(mem << 20);
	else
		realmem = btoc(32 << 20);

	for (i = 0; i < 10; i++) {
		phys_avail[i] = 0;
	}

	/* phys_avail regions are in bytes */
	phys_avail[0] = MIPS_KSEG0_TO_PHYS(kernel_kseg0_end);
	phys_avail[1] = ctob(realmem);

	dump_avail[0] = phys_avail[0];
	dump_avail[1] = phys_avail[1];

	physmem = realmem;

	/* 
	 * ns8250 uart code uses DELAY so ticker should be inititalized 
	 * before cninit. And tick_init_params refers to hz, so * init_param1 
	 * should be called first.
	 */
	init_param1();
	/* TODO: parse argc,argv */
	platform_counter_freq = 330000000UL;
	mips_timer_init_params(platform_counter_freq, 1);
	cninit();
	/* Panic here, after cninit */ 
	if (mem == 0)
		panic("No mem=XX parameter in arguments");

	printf("cmd line: ");
	for (i=0; i < argc; i++)
		printf("%s ", argv[i]);
	printf("\n");

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