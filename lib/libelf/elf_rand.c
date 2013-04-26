
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

off_t
elf_rand(Elf *ar, off_t offset)
{
	struct ar_hdr *arh;

	if (ar == NULL || ar->e_kind != ELF_K_AR ||
	    (offset & 1) || offset < SARMAG ||
	    offset + sizeof(struct ar_hdr) >= ar->e_rawsize) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return 0;
	}

	arh = (struct ar_hdr *) (ar->e_rawfile + offset);

	/* a too simple sanity check */
	if (arh->ar_fmag[0] != '`' || arh->ar_fmag[1] != '\n') {
		LIBELF_SET_ERROR(ARCHIVE, 0);
		return 0;
	}

	ar->e_u.e_ar.e_next = offset;

	return (offset);
}