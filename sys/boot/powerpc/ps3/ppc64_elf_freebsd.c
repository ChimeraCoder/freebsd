
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
#include <sys/linker.h>

#include <machine/metadata.h>
#include <machine/elf.h>

#include <stand.h>

#include "bootstrap.h"

extern char		end[];
extern vm_offset_t	reloc;	/* From <arch>/conf.c */

int
ppc64_elf_loadfile(char *filename, u_int64_t dest,
    struct preloaded_file **result)
{
	int	r;

	r = __elfN(loadfile)(filename, dest, result);
	if (r != 0)
		return (r);

	/*
	 * No need to sync the icache for modules: this will
	 * be done by the kernel after relocation.
	 */
	if (!strcmp((*result)->f_type, "elf kernel"))
		__syncicache((void *) (*result)->f_addr, (*result)->f_size);
	return (0);
}

int
ppc64_elf_exec(struct preloaded_file *fp)
{
	struct file_metadata	*fmp;
	vm_offset_t		mdp;
	Elf_Ehdr		*e;
	int			error;
	int (*entry)(u_long, u_long, u_long, void *, u_long);

	if ((fmp = file_findmetadata(fp, MODINFOMD_ELFHDR)) == NULL) {
		return(EFTYPE);
	}
	e = (Elf_Ehdr *)&fmp->md_data;
	
	/* Handle function descriptor */
	entry = (void *)(uintptr_t)(*(uint64_t *)e->e_entry);

	if ((error = md_load64(fp->f_args, &mdp)) != 0)
		return (error);

	printf("Kernel entry at %p ...\n", entry);

	dev_cleanup();

	entry(0 /* FDT */, 0 /* Phys. mem offset */, 0 /* OF entry */,
	     (void *)mdp, sizeof(mdp));

	panic("exec returned");
}

struct file_format	ppc_elf64 =
{
	ppc64_elf_loadfile,
	ppc64_elf_exec
};