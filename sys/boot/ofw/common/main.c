
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
#include "openfirm.h"
#include "libofw.h"
#include "bootstrap.h"

struct arch_switch	archsw;		/* MI/MD interface boundary */

extern char end[];
extern char bootprog_name[];
extern char bootprog_rev[];
extern char bootprog_date[];
extern char bootprog_maker[];

u_int32_t	acells, scells;

static char bootargs[128];

#define	HEAP_SIZE	0x80000

#define OF_puts(fd, text) OF_write(fd, text, strlen(text))

void
init_heap(void)
{
	void	*base;
	ihandle_t stdout;

	if ((base = ofw_alloc_heap(HEAP_SIZE)) == (void *)0xffffffff) {
		OF_getprop(chosen, "stdout", &stdout, sizeof(stdout));
		OF_puts(stdout, "Heap memory claim failed!\n");
		OF_enter();
	}

	setheap(base, (void *)((int)base + HEAP_SIZE));
}

uint64_t
memsize(void)
{
	phandle_t	memoryp;
	cell_t		reg[24];
	int		i, sz;
	u_int64_t	memsz;

	memsz = 0;
	memoryp = OF_instance_to_package(memory);

	sz = OF_getprop(memoryp, "reg", &reg, sizeof(reg));
	sz /= sizeof(reg[0]);

	for (i = 0; i < sz; i += (acells + scells)) {
		if (scells > 1)
			memsz += (uint64_t)reg[i + acells] << 32;
		memsz += reg[i + acells + scells - 1];
	}

	return (memsz);
}

int
main(int (*openfirm)(void *))
{
	phandle_t	root;
	int		i;
	char		bootpath[64];
	char		*ch;
	int		bargc;
	char		**bargv;

	/*
	 * Initalise the Open Firmware routines by giving them the entry point.
	 */
	OF_init(openfirm);

	root = OF_finddevice("/");

	scells = acells = 1;
	OF_getprop(root, "#address-cells", &acells, sizeof(acells));
	OF_getprop(root, "#size-cells", &scells, sizeof(scells));

	/*
	 * Initialise the heap as early as possible.  Once this is done,
	 * alloc() is usable. The stack is buried inside us, so this is
	 * safe.
	 */
	init_heap();

	/*
         * Set up console.
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
	printf("Memory: %lldKB\n", memsize() / 1024);

	OF_getprop(chosen, "bootpath", bootpath, 64);
	ch = strchr(bootpath, ':');
	*ch = '\0';
	printf("Booted from: %s\n", bootpath);

	printf("\n");

	/*
	 * Only parse the first bootarg if present. It should
	 * be simple to handle extra arguments
	 */
	OF_getprop(chosen, "bootargs", bootargs, sizeof(bootargs));
	bargc = 0;
	parse(&bargc, &bargv, bootargs);
	if (bargc == 1)
		env_setenv("currdev", EV_VOLATILE, bargv[0], ofw_setcurrdev,
		    env_nounset);
	else
		env_setenv("currdev", EV_VOLATILE, bootpath,
			   ofw_setcurrdev, env_nounset);
	env_setenv("loaddev", EV_VOLATILE, bootpath, env_noset,
	    env_nounset);
	setenv("LINES", "24", 1);		/* optional */

	archsw.arch_getdev = ofw_getdev;
	archsw.arch_copyin = ofw_copyin;
	archsw.arch_copyout = ofw_copyout;
	archsw.arch_readin = ofw_readin;
	archsw.arch_autoload = ofw_autoload;

	interact();				/* doesn't return */

	OF_exit();

	return 0;
}

COMMAND_SET(halt, "halt", "halt the system", command_halt);

static int
command_halt(int argc, char *argv[])
{

	OF_exit();
	return (CMD_OK);
}

COMMAND_SET(memmap, "memmap", "print memory map", command_memmap);

int
command_memmap(int argc, char **argv)
{

	ofw_memmap(acells);
	return (CMD_OK);
}