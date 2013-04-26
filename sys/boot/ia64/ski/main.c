
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

#include <stand.h>
#include <string.h>
#include <setjmp.h>
#include <machine/fpu.h>

#include <libia64.h>
#include "libski.h"

extern char bootprog_name[];
extern char bootprog_rev[];
extern char bootprog_date[];
extern char bootprog_maker[];

struct devdesc currdev;		/* our current device */
struct arch_switch archsw;	/* MI/MD interface boundary */

void
ski_main(void)
{
	static char malloc[512*1024];
	int i;

	/* 
	 * initialise the heap as early as possible.  Once this is done,
	 * alloc() is usable. The stack is buried inside us, so this is
	 * safe.
	 */
	setheap((void *)malloc, (void *)(malloc + 512*1024));

	/* 
	 * XXX Chicken-and-egg problem; we want to have console output
	 * early, but some console attributes may depend on reading from
	 * eg. the boot device, which we can't do yet.  We can use
	 * printf() etc. once this is done.
	 */
	cons_probe();

	/*
	 * March through the device switch probing for things.
	 */
	for (i = 0; devsw[i] != NULL; i++)
		if (devsw[i]->dv_init != NULL)
			(devsw[i]->dv_init)();

	printf("\n");
	printf("%s, Revision %s\n", bootprog_name, bootprog_rev);
	printf("(%s, %s)\n", bootprog_maker, bootprog_date);
#if 0
	printf("Memory: %ld k\n", memsize() / 1024);
#endif

	/* XXX presumes that biosdisk is first in devsw */
	currdev.d_dev = devsw[0];
	currdev.d_type = currdev.d_dev->dv_type;
	currdev.d_unit = 0;

#if 0
	/* Create arc-specific variables */
	bootfile = GetEnvironmentVariable(ARCENV_BOOTFILE);
	if (bootfile)
		setenv("bootfile", bootfile, 1);
#endif

	env_setenv("currdev", EV_VOLATILE, ia64_fmtdev(&currdev),
	    ia64_setcurrdev, env_nounset);
	env_setenv("loaddev", EV_VOLATILE, ia64_fmtdev(&currdev), env_noset,
	    env_nounset);

	setenv("LINES", "24", 1);	/* optional */

	archsw.arch_autoload = ia64_autoload;
	archsw.arch_copyin = ia64_copyin;
	archsw.arch_copyout = ia64_copyout;
	archsw.arch_getdev = ia64_getdev;
	archsw.arch_loadaddr = ia64_loadaddr;
	archsw.arch_loadseg = ia64_loadseg;
	archsw.arch_readin = ia64_readin;

	interact();			/* doesn't return */

	exit(0);
}

COMMAND_SET(quit, "quit", "exit the loader", command_quit);

static int
command_quit(int argc, char *argv[])
{
	exit(0);
	return (CMD_OK);
}