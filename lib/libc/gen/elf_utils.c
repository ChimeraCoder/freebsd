
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

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <link.h>
#include <stddef.h>

int
__elf_phdr_match_addr(struct dl_phdr_info *phdr_info, void *addr)
{
	const Elf_Phdr *ph;
	int i;

	for (i = 0; i < phdr_info->dlpi_phnum; i++) {
		ph = &phdr_info->dlpi_phdr[i];
		if (ph->p_type != PT_LOAD || (ph->p_flags & PF_X) == 0)
			continue;
		if (phdr_info->dlpi_addr + ph->p_vaddr <= (uintptr_t)addr &&
		    (uintptr_t)addr + sizeof(addr) < phdr_info->dlpi_addr +
		    ph->p_vaddr + ph->p_memsz)
			break;
	}
	return (i != phdr_info->dlpi_phnum);
}

#pragma weak __pthread_map_stacks_exec
void
__pthread_map_stacks_exec(void)
{
	int mib[2];
	struct rlimit rlim;
	u_long usrstack;
	size_t len;
	
	mib[0] = CTL_KERN;
	mib[1] = KERN_USRSTACK;
	len = sizeof(usrstack);
	if (sysctl(mib, sizeof(mib) / sizeof(mib[0]), &usrstack, &len, NULL, 0)
	    == -1)
		return;
	if (getrlimit(RLIMIT_STACK, &rlim) == -1)
		return;
	mprotect((void *)(uintptr_t)(usrstack - rlim.rlim_cur),
	    rlim.rlim_cur, _rtld_get_stack_prot());
}