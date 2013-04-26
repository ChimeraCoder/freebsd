
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

#define __ELF_WORD_SIZE 64
#include <sys/param.h>
#include <sys/exec.h>
#include <sys/linker.h>
#include <string.h>
#include <machine/bootinfo.h>
#include <machine/elf.h>
#include <stand.h>

#include "bootstrap.h"
#include "libi386.h"
#include "btxv86.h"

static int	elf64_exec(struct preloaded_file *amp);
static int	elf64_obj_exec(struct preloaded_file *amp);

struct file_format amd64_elf = { elf64_loadfile, elf64_exec };
struct file_format amd64_elf_obj = { elf64_obj_loadfile, elf64_obj_exec };

#define PG_V	0x001
#define PG_RW	0x002
#define PG_U	0x004
#define PG_PS	0x080

typedef u_int64_t p4_entry_t;
typedef u_int64_t p3_entry_t;
typedef u_int64_t p2_entry_t;
extern p4_entry_t PT4[];
extern p3_entry_t PT3[];
extern p2_entry_t PT2[];

u_int32_t entry_hi;
u_int32_t entry_lo;

extern void amd64_tramp();

/*
 * There is an ELF kernel and one or more ELF modules loaded.  
 * We wish to start executing the kernel image, so make such 
 * preparations as are required, and do so.
 */
static int
elf64_exec(struct preloaded_file *fp)
{
    struct file_metadata	*md;
    Elf_Ehdr 			*ehdr;
    vm_offset_t			modulep, kernend;
    int				err;
    int				i;

    if ((md = file_findmetadata(fp, MODINFOMD_ELFHDR)) == NULL)
	return(EFTYPE);
    ehdr = (Elf_Ehdr *)&(md->md_data);

    err = bi_load64(fp->f_args, &modulep, &kernend);
    if (err != 0)
	return(err);

    bzero(PT4, PAGE_SIZE);
    bzero(PT3, PAGE_SIZE);
    bzero(PT2, PAGE_SIZE);

    /*
     * This is kinda brutal, but every single 1GB VM memory segment points to
     * the same first 1GB of physical memory.  But it is more than adequate.
     */
    for (i = 0; i < 512; i++) {
	/* Each slot of the level 4 pages points to the same level 3 page */
	PT4[i] = (p4_entry_t)VTOP((uintptr_t)&PT3[0]);
	PT4[i] |= PG_V | PG_RW | PG_U;

	/* Each slot of the level 3 pages points to the same level 2 page */
	PT3[i] = (p3_entry_t)VTOP((uintptr_t)&PT2[0]);
	PT3[i] |= PG_V | PG_RW | PG_U;

	/* The level 2 page slots are mapped with 2MB pages for 1GB. */
	PT2[i] = i * (2 * 1024 * 1024);
	PT2[i] |= PG_V | PG_RW | PG_PS | PG_U;
    }

    entry_lo = ehdr->e_entry & 0xffffffff;
    entry_hi = (ehdr->e_entry >> 32) & 0xffffffff;
#ifdef DEBUG
    printf("Start @ %#llx ...\n", ehdr->e_entry);
#endif

    dev_cleanup();
    __exec((void *)VTOP(amd64_tramp), modulep, kernend);

    panic("exec returned");
}

static int
elf64_obj_exec(struct preloaded_file *fp)
{
	return (EFTYPE);
}