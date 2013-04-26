
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

#include <sys/mman.h>

#include <assert.h>
#include <libelf.h>
#include <stdlib.h>

#include "_libelf.h"

int
elf_end(Elf *e)
{
	Elf *sv;
	Elf_Scn *scn, *tscn;

	if (e == NULL || e->e_activations == 0)
		return (0);

	if (--e->e_activations > 0)
		return (e->e_activations);

	assert(e->e_activations == 0);

	while (e && e->e_activations == 0) {
		switch (e->e_kind) {
		case ELF_K_AR:
			/*
			 * If we still have open child descriptors, we
			 * need to defer reclaiming resources till all
			 * the child descriptors for the archive are
			 * closed.
			 */
			if (e->e_u.e_ar.e_nchildren > 0)
				return (0);
			break;
		case ELF_K_ELF:
			/*
			 * Reclaim all section descriptors.
			 */
			STAILQ_FOREACH_SAFE(scn, &e->e_u.e_elf.e_scn, s_next, tscn)
 				scn = _libelf_release_scn(scn);
			break;
		case ELF_K_NUM:
			assert(0);
		default:
			break;
		}

		if (e->e_flags & LIBELF_F_MMAP)
			(void) munmap(e->e_rawfile, e->e_rawsize);

		sv = e;
		if ((e = e->e_parent) != NULL)
			e->e_u.e_ar.e_nchildren--;
		sv = _libelf_release_elf(sv);
	}

	return (0);
}