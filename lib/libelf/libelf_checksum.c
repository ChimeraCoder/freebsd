
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

#include <gelf.h>

#include "_libelf.h"

static unsigned long
_libelf_sum(unsigned long c, const unsigned char *s, size_t size)
{
	if (s == NULL || size == 0)
		return (c);

	while (size--)
		c += *s++;

	return (c);
}

unsigned long
_libelf_checksum(Elf *e, int elfclass)
{
	size_t shn;
	Elf_Scn *scn;
	Elf_Data *d;
	unsigned long checksum;
	GElf_Ehdr eh;
	GElf_Shdr shdr;

	if (e == NULL) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0L);
	}

	if (e->e_class != elfclass) {
		LIBELF_SET_ERROR(CLASS, 0);
		return (0L);
	}

	if (gelf_getehdr(e, &eh) == NULL)
		return (0);

	/*
	 * Iterate over all sections in the ELF file, computing the
	 * checksum along the way.
	 *
	 * The first section is always SHN_UNDEF and can be skipped.
	 * Non-allocatable sections are skipped, as are sections that
	 * could be affected by utilities such as strip(1).
	 */

	checksum = 0;
	for (shn = 1; shn < e->e_u.e_elf.e_nscn; shn++) {
		if ((scn = elf_getscn(e, shn)) == NULL)
			return (0);
		if (gelf_getshdr(scn, &shdr) == NULL)
			return (0);
		if ((shdr.sh_flags & SHF_ALLOC) == 0 ||
		    shdr.sh_type == SHT_DYNAMIC ||
		    shdr.sh_type == SHT_DYNSYM)
			continue;

		d = NULL;
		while ((d = elf_rawdata(scn, d)) != NULL)
			checksum = _libelf_sum(checksum,
			    (unsigned char *) d->d_buf, d->d_size);
	}

	/*
	 * Return a 16-bit checksum compatible with Solaris.
	 */
	return (((checksum >> 16) & 0xFFFFUL) + (checksum & 0xFFFFUL));
}