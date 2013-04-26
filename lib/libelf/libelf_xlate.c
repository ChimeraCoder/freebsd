
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

#include <assert.h>
#include <libelf.h>

#include "_libelf.h"

/*
 * Translate to/from the file representation of ELF objects.
 *
 * Translation could potentially involve the following
 * transformations:
 *
 * - an endianness conversion,
 * - a change of layout, as the file representation of ELF objects
 *   can differ from their in-memory representation.
 * - a change in representation due to a layout version change.
 */

Elf_Data *
_libelf_xlate(Elf_Data *dst, const Elf_Data *src, unsigned int encoding,
    int elfclass, int direction)
{
	int byteswap;
	size_t cnt, dsz, fsz, msz;
	uintptr_t sb, se, db, de;

	if (encoding == ELFDATANONE)
		encoding = LIBELF_PRIVATE(byteorder);

	if ((encoding != ELFDATA2LSB && encoding != ELFDATA2MSB) ||
	    dst == NULL || src == NULL || dst == src)	{
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	assert(elfclass == ELFCLASS32 || elfclass == ELFCLASS64);
	assert(direction == ELF_TOFILE || direction == ELF_TOMEMORY);

	if (dst->d_version != src->d_version) {
		LIBELF_SET_ERROR(UNIMPL, 0);
		return (NULL);
	}

	if  (src->d_buf == NULL || dst->d_buf == NULL) {
		LIBELF_SET_ERROR(DATA, 0);
		return (NULL);
	}

	if ((int) src->d_type < 0 || src->d_type >= ELF_T_NUM) {
		LIBELF_SET_ERROR(DATA, 0);
		return (NULL);
	}

	if ((fsz = (elfclass == ELFCLASS32 ? elf32_fsize : elf64_fsize)
	    (src->d_type, (size_t) 1, src->d_version)) == 0)
		return (NULL);

	msz = _libelf_msize(src->d_type, elfclass, src->d_version);

	assert(msz > 0);

	if (src->d_size % (direction == ELF_TOMEMORY ? fsz : msz)) {
		LIBELF_SET_ERROR(DATA, 0);
		return (NULL);
	}

	/*
	 * Determine the number of objects that need to be converted, and
	 * the space required for the converted objects in the destination
	 * buffer.
	 */
	if (direction == ELF_TOMEMORY) {
		cnt = src->d_size / fsz;
		dsz = cnt * msz;
	} else {
		cnt = src->d_size / msz;
		dsz = cnt * fsz;
	}

	if (dst->d_size  <  dsz) {
		LIBELF_SET_ERROR(DATA, 0);
		return (NULL);
	}

	sb = (uintptr_t) src->d_buf;
	se = sb + src->d_size;
	db = (uintptr_t) dst->d_buf;
	de = db + dst->d_size;

	/*
	 * Check for overlapping buffers.  Note that db == sb is
	 * allowed.
	 */
	if (db != sb && de > sb && se > db) {
		LIBELF_SET_ERROR(DATA, 0);
		return (NULL);
	}

	if ((direction == ELF_TOMEMORY ? db : sb) %
	    _libelf_malign(src->d_type, elfclass)) {
		LIBELF_SET_ERROR(DATA, 0);
		return (NULL);
	}

	dst->d_type = src->d_type;
	dst->d_size = dsz;

	byteswap = encoding != LIBELF_PRIVATE(byteorder);

	if (src->d_size == 0 ||
	    (db == sb && !byteswap && fsz == msz))
		return (dst);	/* nothing more to do */

	if (!(_libelf_get_translator(src->d_type, direction, elfclass))
	    (dst->d_buf, dsz, src->d_buf, cnt, byteswap)) {
		LIBELF_SET_ERROR(DATA, 0);
		return (NULL);
	}

	return (dst);
}