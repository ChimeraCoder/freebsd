
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
#include <libelf.h>

#include "_libelf.h"

static int
_libelf_getphdrnum(Elf *e, size_t *phnum)
{
	void *eh;
	int ec;

	if (e == NULL || e->e_kind != ELF_K_ELF ||
	    ((ec = e->e_class) != ELFCLASS32 && ec != ELFCLASS64)) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (-1);
	}

	if ((eh = _libelf_ehdr(e, ec, 0)) == NULL)
		return (-1);

	*phnum = e->e_u.e_elf.e_nphdr;

	return (0);
}

int
elf_getphdrnum(Elf *e, size_t *phnum)
{
	return (_libelf_getphdrnum(e, phnum));
}

/* Deprecated API */
int
elf_getphnum(Elf *e, size_t *phnum)
{
	return (_libelf_getphdrnum(e, phnum) >= 0);
}