
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

#include <mips/rt305x/rt305xreg.h>

extern int	*edata;
extern int	*end;
static char 	boot1_env[0x1000];


void
platform_cpu_init()
{
	/* Nothing special */
}

static void
mips_init(void)
{
	int i;

	printf("entry: mips_init()\n");

	bootverbose = 1;
	realmem = btoc(32 << 20);

	for (i = 0; i < 10; i++) {
		phys_avail[i] = 0;
	}

	/* phys_avail regions are in bytes */
	dump_avail[0] = phys_avail[0] = MIPS_KSEG0_TO_PHYS(kernel_kseg0_end);
	dump_avail[1] = phys_avail[1] = ctob(realmem);

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

void
platform_reset(void)
{

	__asm __volatile("li	$25, 0xbf000000");
	__asm __volatile("j	$25");
}

void
platform_start(__register_t a0 __unused, __register_t a1 __unused, 
    __register_t a2 __unused, __register_t a3 __unused)
{
	vm_offset_t kernend;
	uint64_t platform_counter_freq = PLATFORM_COUNTER_FREQ;
	int i;
	int argc = a0;
	char **argv = (char **)MIPS_PHYS_TO_KSEG0(a1);
	char **envp = (char **)MIPS_PHYS_TO_KSEG0(a2);

	/* clear the BSS and SBSS segments */
	kernend = (vm_offset_t)&end;
	memset(&edata, 0, kernend - (vm_offset_t)(&edata));

	mips_postboot_fixup();

	/* Initialize pcpu stuff */
	mips_pcpu0_init();

	/* initialize console so that we have printf */
	boothowto |= (RB_SERIAL | RB_MULTIPLE);	/* Use multiple consoles */
	boothowto |= (RB_VERBOSE);
	cninit();

	init_static_kenv(boot1_env, sizeof(boot1_env));

	printf("U-Boot args (from %d args):\n", argc - 1);

	if (argc == 1)
		printf("\tNone\n");

	for (i = 1; i < argc; i++) {
		char *n = "argv  ", *arg;

		if (i > 99)
			break;

		if (argv[i])
		{
			arg = (char *)(intptr_t)MIPS_PHYS_TO_KSEG0(argv[i]);
			printf("\targv[%d] = %s\n", i, arg);
			sprintf(n, "argv%d", i);
			setenv(n, arg);
		}
	}

	printf("Environment:\n");

	for (i = 0; envp[i] ; i++) {
		char *n, *arg;

		arg = (char *)(intptr_t)MIPS_PHYS_TO_KSEG0(envp[i]);
		printf("\t%s\n", arg);
		n = strsep(&arg, "=");
		if (arg == NULL)
			setenv(n, "1");
		else
			setenv(n, arg);
	}


	mips_init();
	mips_timer_init_params(platform_counter_freq, 2);
}