
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
#include <sys/linker.h>

#include <machine/metadata.h>
#include <machine/elf.h>

#include <stand.h>

#include "bootstrap.h"
#include "libofw.h"
#include "openfirm.h"

extern char		end[];
extern vm_offset_t	reloc;	/* From <arch>/conf.c */

int
__elfN(ofw_loadfile)(char *filename, u_int64_t dest,
    struct preloaded_file **result)
{
	int	r;

	r = __elfN(loadfile)(filename, dest, result);
	if (r != 0)
		return (r);

#if defined(__powerpc__)
	/*
	 * No need to sync the icache for modules: this will
	 * be done by the kernel after relocation.
	 */
	if (!strcmp((*result)->f_type, "elf kernel"))
		__syncicache((void *) (*result)->f_addr, (*result)->f_size);
#endif
	return (0);
}

int
__elfN(ofw_exec)(struct preloaded_file *fp)
{
	struct file_metadata	*fmp;
	vm_offset_t		mdp;
	Elf_Ehdr		*e;
	int			error;
	intptr_t		entry;

	if ((fmp = file_findmetadata(fp, MODINFOMD_ELFHDR)) == NULL) {
		return(EFTYPE);
	}
	e = (Elf_Ehdr *)&fmp->md_data;
	entry = e->e_entry;

	if ((error = md_load(fp->f_args, &mdp)) != 0)
		return (error);

	printf("Kernel entry at 0x%lx ...\n", e->e_entry);

	dev_cleanup();
	ofw_release_heap();
	OF_chain((void *)reloc, end - (char *)reloc, (void *)entry,
	    (void *)mdp, sizeof(mdp));

	panic("exec returned");
}

struct file_format	ofw_elf =
{
	__elfN(ofw_loadfile),
	__elfN(ofw_exec)
};