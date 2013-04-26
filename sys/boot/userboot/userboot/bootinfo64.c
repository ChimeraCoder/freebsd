
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
#include <sys/param.h>
#include <sys/reboot.h>
#include <sys/linker.h>
#include <i386/include/bootinfo.h>
#include <machine/cpufunc.h>
#include <machine/psl.h>
#include <machine/specialreg.h>

#include "bootstrap.h"
#include "libuserboot.h"

/*
 * Copy module-related data into the load area, where it can be
 * used as a directory for loaded modules.
 *
 * Module data is presented in a self-describing format.  Each datum
 * is preceded by a 32-bit identifier and a 32-bit size field.
 *
 * Currently, the following data are saved:
 *
 * MOD_NAME	(variable)		module name (string)
 * MOD_TYPE	(variable)		module type (string)
 * MOD_ARGS	(variable)		module parameters (string)
 * MOD_ADDR	sizeof(vm_offset_t)	module load address
 * MOD_SIZE	sizeof(size_t)		module size
 * MOD_METADATA	(variable)		type-specific metadata
 */
#define COPY32(v, a, c) {			\
    u_int32_t	x = (v);			\
    if (c)					\
        CALLBACK(copyin, &x, a, sizeof(x));	\
    a += sizeof(x);				\
}

#define MOD_STR(t, a, s, c) {			\
    COPY32(t, a, c);				\
    COPY32(strlen(s) + 1, a, c);		\
    if (c)					\
        CALLBACK(copyin, s, a, strlen(s) + 1);  \
    a += roundup(strlen(s) + 1, sizeof(u_int64_t));\
}

#define MOD_NAME(a, s, c)	MOD_STR(MODINFO_NAME, a, s, c)
#define MOD_TYPE(a, s, c)	MOD_STR(MODINFO_TYPE, a, s, c)
#define MOD_ARGS(a, s, c)	MOD_STR(MODINFO_ARGS, a, s, c)

#define MOD_VAR(t, a, s, c) {			\
    COPY32(t, a, c);				\
    COPY32(sizeof(s), a, c);			\
    if (c)					\
        CALLBACK(copyin, &s, a, sizeof(s));	\
    a += roundup(sizeof(s), sizeof(u_int64_t));	\
}

#define MOD_ADDR(a, s, c)	MOD_VAR(MODINFO_ADDR, a, s, c)
#define MOD_SIZE(a, s, c)	MOD_VAR(MODINFO_SIZE, a, s, c)

#define MOD_METADATA(a, mm, c) {		\
    COPY32(MODINFO_METADATA | mm->md_type, a, c); \
    COPY32(mm->md_size, a, c);			\
    if (c)					\
        CALLBACK(copyin, mm->md_data, a, mm->md_size);    \
    a += roundup(mm->md_size, sizeof(u_int64_t));\
}

#define MOD_END(a, c) {				\
    COPY32(MODINFO_END, a, c);			\
    COPY32(0, a, c);				\
}

static vm_offset_t
bi_copymodules64(vm_offset_t addr)
{
    struct preloaded_file	*fp;
    struct file_metadata	*md;
    int				c;
    u_int64_t			v;

    c = addr != 0;
    /* start with the first module on the list, should be the kernel */
    for (fp = file_findfile(NULL, NULL); fp != NULL; fp = fp->f_next) {

	MOD_NAME(addr, fp->f_name, c);	/* this field must come first */
	MOD_TYPE(addr, fp->f_type, c);
	if (fp->f_args)
	    MOD_ARGS(addr, fp->f_args, c);
	v = fp->f_addr;
	MOD_ADDR(addr, v, c);
	v = fp->f_size;
	MOD_SIZE(addr, v, c);
	for (md = fp->f_metadata; md != NULL; md = md->md_next)
	    if (!(md->md_type & MODINFOMD_NOCOPY))
		MOD_METADATA(addr, md, c);
    }
    MOD_END(addr, c);
    return(addr);
}

/*
 * Check to see if this CPU supports long mode.
 */
static int
bi_checkcpu(void)
{
#if 0
    char *cpu_vendor;
    int vendor[3];
    int eflags, regs[4];

    /* Check for presence of "cpuid". */
    eflags = read_eflags();
    write_eflags(eflags ^ PSL_ID);
    if (!((eflags ^ read_eflags()) & PSL_ID))
	return (0);

    /* Fetch the vendor string. */
    do_cpuid(0, regs);
    vendor[0] = regs[1];
    vendor[1] = regs[3];
    vendor[2] = regs[2];
    cpu_vendor = (char *)vendor;

    /* Check for vendors that support AMD features. */
    if (strncmp(cpu_vendor, INTEL_VENDOR_ID, 12) != 0 &&
	strncmp(cpu_vendor, AMD_VENDOR_ID, 12) != 0 &&
	strncmp(cpu_vendor, CENTAUR_VENDOR_ID, 12) != 0)
	return (0);

    /* Has to support AMD features. */
    do_cpuid(0x80000000, regs);
    if (!(regs[0] >= 0x80000001))
	return (0);

    /* Check for long mode. */
    do_cpuid(0x80000001, regs);
    return (regs[3] & AMDID_LM);
#else
	return (1);
#endif
}

struct smap {
        uint64_t       base;
        uint64_t       length;
        uint32_t       type;
} __packed;

/* From FreeBSD <machine/pc/bios.h> */
#define SMAP_TYPE_MEMORY	1

#define GB (1024UL * 1024 * 1024)

#define MODINFOMD_SMAP          0x1001

static void
bios_addsmapdata(struct preloaded_file *kfp)
{
	uint64_t lowmem, highmem;
	int smapnum, len;
	struct smap smap[3], *sm;

	CALLBACK(getmem, &lowmem, &highmem);

	sm = &smap[0];

	sm->base = 0;				/* base memory */
	sm->length = 640 * 1024;
	sm->type = SMAP_TYPE_MEMORY;
	sm++;

	sm->base = 0x100000;			/* extended memory */
	sm->length = lowmem - 0x100000;
	sm->type = SMAP_TYPE_MEMORY;
	sm++;

	smapnum = 2;

        if (highmem != 0) {
                sm->base = 4 * GB;
                sm->length = highmem;
                sm->type = SMAP_TYPE_MEMORY;
		smapnum++;
        }

        len = smapnum * sizeof (struct smap);
        file_addmetadata(kfp, MODINFOMD_SMAP, len, &smap[0]);
}

/*
 * Load the information expected by an amd64 kernel.
 *
 * - The 'boothowto' argument is constructed
 * - The 'bootdev' argument is constructed
 * - The 'bootinfo' struct is constructed, and copied into the kernel space.
 * - The kernel environment is copied into kernel space.
 * - Module metadata are formatted and placed in kernel space.
 */
int
bi_load64(char *args, vm_offset_t *modulep, vm_offset_t *kernendp)
{
    struct preloaded_file	*xp, *kfp;
    struct userboot_devdesc	*rootdev;
    struct file_metadata	*md;
    vm_offset_t			addr;
    u_int64_t			kernend;
    u_int64_t			envp;
    vm_offset_t			size;
    char			*rootdevname;
    int				howto;

    if (!bi_checkcpu()) {
	printf("CPU doesn't support long mode\n");
	return (EINVAL);
    }

    howto = bi_getboothowto(args);

    /* 
     * Allow the environment variable 'rootdev' to override the supplied device 
     * This should perhaps go to MI code and/or have $rootdev tested/set by
     * MI code before launching the kernel.
     */
    rootdevname = getenv("rootdev");
    userboot_getdev((void **)(&rootdev), rootdevname, NULL);
    if (rootdev == NULL) {		/* bad $rootdev/$currdev */
	printf("can't determine root device\n");
	return(EINVAL);
    }

    /* Try reading the /etc/fstab file to select the root device */
    getrootmount(userboot_fmtdev((void *)rootdev));

    /* find the last module in the chain */
    addr = 0;
    for (xp = file_findfile(NULL, NULL); xp != NULL; xp = xp->f_next) {
	if (addr < (xp->f_addr + xp->f_size))
	    addr = xp->f_addr + xp->f_size;
    }
    /* pad to a page boundary */
    addr = roundup(addr, PAGE_SIZE);

    /* copy our environment */
    envp = addr;
    addr = bi_copyenv(addr);

    /* pad to a page boundary */
    addr = roundup(addr, PAGE_SIZE);

    kfp = file_findfile(NULL, "elf kernel");
    if (kfp == NULL)
      kfp = file_findfile(NULL, "elf64 kernel");
    if (kfp == NULL)
	panic("can't find kernel file");
    kernend = 0;	/* fill it in later */
    file_addmetadata(kfp, MODINFOMD_HOWTO, sizeof howto, &howto);
    file_addmetadata(kfp, MODINFOMD_ENVP, sizeof envp, &envp);
    file_addmetadata(kfp, MODINFOMD_KERNEND, sizeof kernend, &kernend);
    bios_addsmapdata(kfp);

    /* Figure out the size and location of the metadata */
    *modulep = addr;
    size = bi_copymodules64(0);
    kernend = roundup(addr + size, PAGE_SIZE);
    *kernendp = kernend;

    /* patch MODINFOMD_KERNEND */
    md = file_findmetadata(kfp, MODINFOMD_KERNEND);
    bcopy(&kernend, md->md_data, sizeof kernend);

    /* copy module list and metadata */
    (void)bi_copymodules64(addr);

    return(0);
}