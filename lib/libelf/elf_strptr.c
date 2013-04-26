
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

#include <assert.h>
#include <gelf.h>

#include "_libelf.h"

/*
 * Convert an ELF section#,offset pair to a string pointer.
 */

char *
elf_strptr(Elf *e, size_t scndx, size_t offset)
{
	Elf_Scn *s;
	Elf_Data *d;
	size_t alignment, count;
	GElf_Shdr shdr;

	if (e == NULL || e->e_kind != ELF_K_ELF) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	if ((s = elf_getscn(e, scndx)) == NULL ||
	    gelf_getshdr(s, &shdr) == NULL)
		return (NULL);

	if (/*shdr.sh_type != SHT_STRTAB || */
	    offset >= shdr.sh_size) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	d = NULL;
	if (e->e_flags & ELF_F_LAYOUT) {

		/*
		 * The application is taking responsibility for the
		 * ELF object's layout, so we can directly translate
		 * an offset to a `char *' address using the `d_off'
		 * members of Elf_Data descriptors.
		 */
		while ((d = elf_getdata(s, d)) != NULL) {

			if (d->d_buf == 0 || d->d_size == 0)
				continue;

			if (d->d_type != ELF_T_BYTE) {
				LIBELF_SET_ERROR(DATA, 0);
				return (NULL);
			}

			if (offset >= d->d_off &&
			    offset < d->d_off + d->d_size)
				return ((char *) d->d_buf + offset - d->d_off);
		}
	} else {
		/*
		 * Otherwise, the `d_off' members are not useable and
		 * we need to compute offsets ourselves, taking into
		 * account 'holes' in coverage of the section introduced
		 * by alignment requirements.
		 */
		count = (size_t) 0;	/* cumulative count of bytes seen */
		while ((d = elf_getdata(s, d)) != NULL && count <= offset) {

			if (d->d_buf == NULL || d->d_size == 0)
				continue;

			if (d->d_type != ELF_T_BYTE) {
				LIBELF_SET_ERROR(DATA, 0);
				return (NULL);
			}

			if ((alignment = d->d_align) > 1) {
				if ((alignment & (alignment - 1)) != 0) {
					LIBELF_SET_ERROR(DATA, 0);
					return (NULL);
				}
				count = roundup2(count, alignment);
			}

			if (offset < count) {
				/* offset starts in the 'hole' */
				LIBELF_SET_ERROR(ARGUMENT, 0);
				return (NULL);
			}

			if (offset < count + d->d_size) {
				if (d->d_buf != NULL)
					return ((char *) d->d_buf +
					    offset - count);
				LIBELF_SET_ERROR(DATA, 0);
				return (NULL);
			}

			count += d->d_size;
		}
	}

	LIBELF_SET_ERROR(ARGUMENT, 0);
	return (NULL);
}