
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

#include <ar.h>
#include <assert.h>
#include <libelf.h>

#include "_libelf.h"

char *
elf_getident(Elf *e, size_t *sz)
{

	if (e == NULL) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		goto error;
	}

	if (e->e_cmd == ELF_C_WRITE && e->e_rawfile == NULL) {
		LIBELF_SET_ERROR(SEQUENCE, 0);
		goto error;
	}

	assert(e->e_kind != ELF_K_AR || e->e_cmd == ELF_C_READ);

	if (sz) {
		if (e->e_kind == ELF_K_AR)
			*sz = SARMAG;
		else if (e->e_kind == ELF_K_ELF)
			*sz = EI_NIDENT;
		else
			*sz = e->e_rawsize;
	}

	return (e->e_rawfile);

 error:
	if (sz)
		*sz = 0;
	return (NULL);
}