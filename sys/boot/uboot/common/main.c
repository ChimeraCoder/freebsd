
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

#include "api_public.h"
#include "bootstrap.h"
#include "glue.h"
#include "libuboot.h"

struct uboot_devdesc currdev;
struct arch_switch archsw;		/* MI/MD interface boundary */
int devs_no;

extern char end[];
extern char bootprog_name[];
extern char bootprog_rev[];
extern char bootprog_date[];
extern char bootprog_maker[];

extern unsigned char _etext[];
extern unsigned char _edata[];
extern unsigned char __bss_start[];
extern unsigned char __sbss_start[];
extern unsigned char __sbss_end[];
extern unsigned char _end[];

#ifdef LOADER_FDT_SUPPORT
extern int command_fdt_internal(int argc, char *argv[]);
#endif

static void
dump_sig(struct api_signature *sig)
{
#ifdef DEBUG
	printf("signature:\n");
	printf("  version\t= %d\n", sig->version);
	printf("  checksum\t= 0x%08x\n", sig->checksum);
	printf("  sc entry\t= 0x%08x\n", sig->syscall);
#endif
}

static void
dump_addr_info(void)
{
#ifdef DEBUG
	printf("\naddresses info:\n");
	printf(" _etext (sdata) = 0x%08x\n", (uint32_t)_etext);
	printf(" _edata         = 0x%08x\n", (uint32_t)_edata);
	printf(" __sbss_start   = 0x%08x\n", (uint32_t)__sbss_start);
	printf(" __sbss_end     = 0x%08x\n", (uint32_t)__sbss_end);
	printf(" __sbss_start   = 0x%08x\n", (uint32_t)__bss_start);
	printf(" _end           = 0x%08x\n", (uint32_t)_end);
	printf(" syscall entry  = 0x%08x\n", (uint32_t)syscall_ptr);
#endif
}

static uint64_t
memsize(struct sys_info *si, int flags)
{
	uint64_t size;
	int i;

	size = 0;
	for (i = 0; i < si->mr_no; i++)
		if (si->mr[i].flags == flags && si->mr[i].size)
			size += (si->mr[i].size);

	return (size);
}

static void
meminfo(void)
{
	uint64_t size;
	struct sys_info *si;
	int t[3] = { MR_ATTR_DRAM, MR_ATTR_FLASH, MR_ATTR_SRAM };
	int i;

	if ((si = ub_get_sys_info()) == NULL)
		panic("could not retrieve system info");

	for (i = 0; i < 3; i++) {
		size = memsize(si, t[i]);
		if (size > 0)
			printf("%s:\t %lldMB\n", ub_mem_type(t[i]),
			    size / 1024 / 1024);
	}
}

int
main(void)
{
	struct api_signature *sig = NULL;
	int i;
	struct open_file f;

	if (!api_search_sig(&sig))
		return (-1);

	syscall_ptr = sig->syscall;
	if (syscall_ptr == NULL)
		return (-2);

	if (sig->version > API_SIG_VERSION)
		return (-3);

        /* Clear BSS sections */
	bzero(__sbss_start, __sbss_end - __sbss_start);
	bzero(__bss_start, _end - __bss_start);

	/*
         * Set up console.
         */
	cons_probe();

	printf("Compatible API signature found @%x\n", (uint32_t)sig);

	dump_sig(sig);
	dump_addr_info();

	/*
	 * Initialise the heap as early as possible.  Once this is done,
	 * alloc() is usable. The stack is buried inside us, so this is
	 * safe.
	 */
	setheap((void *)end, (void *)(end + 512 * 1024));

	/*
	 * Enumerate U-Boot devices
	 */
	if ((devs_no = ub_dev_enum()) == 0)
		panic("no U-Boot devices found");
	printf("Number of U-Boot devices: %d\n", devs_no);

	printf("\n");
	printf("%s, Revision %s\n", bootprog_name, bootprog_rev);
	printf("(%s, %s)\n", bootprog_maker, bootprog_date);
	meminfo();

	/*
	 * March through the device switch probing for things.
	 */
	for (i = 0; devsw[i] != NULL; i++) {

		if (devsw[i]->dv_init == NULL)
			continue;
		if ((devsw[i]->dv_init)() != 0)
			continue;

		printf("\nDevice: %s\n", devsw[i]->dv_name);

		currdev.d_dev = devsw[i];
		currdev.d_type = currdev.d_dev->dv_type;
		currdev.d_unit = 0;

		if (strncmp(devsw[i]->dv_name, "disk",
		    strlen(devsw[i]->dv_name)) == 0) {
			f.f_devdata = &currdev;
			currdev.d_disk.slice = 0;
			if (devsw[i]->dv_open(&f,&currdev) == 0)
				break;
		}

		if (strncmp(devsw[i]->dv_name, "net",
		    strlen(devsw[i]->dv_name)) == 0)
			break;
	}

	if (devsw[i] == NULL)
		panic("No boot device found!");

	env_setenv("currdev", EV_VOLATILE, uboot_fmtdev(&currdev),
	    uboot_setcurrdev, env_nounset);
	env_setenv("loaddev", EV_VOLATILE, uboot_fmtdev(&currdev),
	    env_noset, env_nounset);

	setenv("LINES", "24", 1);		/* optional */
	setenv("prompt", "loader>", 1);

	archsw.arch_getdev = uboot_getdev;
	archsw.arch_copyin = uboot_copyin;
	archsw.arch_copyout = uboot_copyout;
	archsw.arch_readin = uboot_readin;
	archsw.arch_autoload = uboot_autoload;

	interact();				/* doesn't return */

	return (0);
}


COMMAND_SET(heap, "heap", "show heap usage", command_heap);
static int
command_heap(int argc, char *argv[])
{

	printf("heap base at %p, top at %p, used %d\n", end, sbrk(0),
	    sbrk(0) - end);

	return (CMD_OK);
}

COMMAND_SET(reboot, "reboot", "reboot the system", command_reboot);
static int
command_reboot(int argc, char *argv[])
{

	printf("Resetting...\n");
	ub_reset();

	printf("Reset failed!\n");
	while(1);
}

COMMAND_SET(devinfo, "devinfo", "show U-Boot devices", command_devinfo);
static int
command_devinfo(int argc, char *argv[])
{
	int i;

	if ((devs_no = ub_dev_enum()) == 0) {
		command_errmsg = "no U-Boot devices found!?";
		return (CMD_ERROR);
	}
	
	printf("U-Boot devices:\n");
	for (i = 0; i < devs_no; i++) {
		ub_dump_di(i);
		printf("\n");
	}
	return (CMD_OK);
}

COMMAND_SET(sysinfo, "sysinfo", "show U-Boot system info", command_sysinfo);
static int
command_sysinfo(int argc, char *argv[])
{
	struct sys_info *si;

	if ((si = ub_get_sys_info()) == NULL) {
		command_errmsg = "could not retrieve U-Boot sys info!?";
		return (CMD_ERROR);
	}

	printf("U-Boot system info:\n");
	ub_dump_si(si);
	return (CMD_OK);
}

#ifdef LOADER_FDT_SUPPORT
/*
 * Since proper fdt command handling function is defined in fdt_loader_cmd.c,
 * and declaring it as extern is in contradiction with COMMAND_SET() macro
 * (which uses static pointer), we're defining wrapper function, which
 * calls the proper fdt handling routine.
 */
static int
command_fdt(int argc, char *argv[])
{

	return (command_fdt_internal(argc, argv));
}

COMMAND_SET(fdt, "fdt", "flattened device tree handling", command_fdt);
#endif