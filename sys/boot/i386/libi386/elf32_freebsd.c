
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
#include <sys/exec.h>
#include <sys/linker.h>
#include <string.h>
#include <machine/bootinfo.h>
#include <machine/elf.h>
#include <stand.h>

#include "bootstrap.h"
#include "libi386.h"
#include "btxv86.h"

static int	elf32_exec(struct preloaded_file *amp);
static int	elf32_obj_exec(struct preloaded_file *amp);

struct file_format i386_elf = { elf32_loadfile, elf32_exec };
struct file_format i386_elf_obj = { elf32_obj_loadfile, elf32_obj_exec };

/*
 * There is an ELF kernel and one or more ELF modules loaded.  
 * We wish to start executing the kernel image, so make such 
 * preparations as are required, and do so.
 */
static int
elf32_exec(struct preloaded_file *fp)
{
    struct file_metadata	*md;
    Elf_Ehdr 			*ehdr;
    vm_offset_t			entry, bootinfop, modulep, kernend;
    int				boothowto, err, bootdev;

    if ((md = file_findmetadata(fp, MODINFOMD_ELFHDR)) == NULL)
	return(EFTYPE);
    ehdr = (Elf_Ehdr *)&(md->md_data);

    err = bi_load32(fp->f_args, &boothowto, &bootdev, &bootinfop, &modulep, &kernend);
    if (err != 0)
	return(err);
    entry = ehdr->e_entry & 0xffffff;

#ifdef DEBUG
    printf("Start @ 0x%lx ...\n", entry);
#endif

    dev_cleanup();
    __exec((void *)entry, boothowto, bootdev, 0, 0, 0, bootinfop, modulep, kernend);

    panic("exec returned");
}

static int
elf32_obj_exec(struct preloaded_file *fp)
{
	return (EFTYPE);
}