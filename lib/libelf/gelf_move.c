
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

#include <sys/limits.h>

#include <assert.h>
#include <gelf.h>
#include <osreldate.h>

#include "_libelf.h"

#if	__FreeBSD_version >= 700025

GElf_Move *
gelf_getmove(Elf_Data *d, int ndx, GElf_Move *dst)
{
	int ec;
	Elf *e;
	Elf_Scn *scn;
	Elf32_Move *move32;
	Elf64_Move *move64;
	size_t msz;
	uint32_t sh_type;

	if (d == NULL || ndx < 0 || dst == NULL ||
	    (scn = d->d_scn) == NULL ||
	    (e = scn->s_elf) == NULL) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	ec = e->e_class;
	assert(ec == ELFCLASS32 || ec == ELFCLASS64);

	if (ec == ELFCLASS32)
		sh_type = scn->s_shdr.s_shdr32.sh_type;
	else
		sh_type = scn->s_shdr.s_shdr64.sh_type;

	if (_libelf_xlate_shtype(sh_type) != ELF_T_MOVE) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	msz = _libelf_msize(ELF_T_MOVE, ec, e->e_version);

	assert(msz > 0);

	if (msz * ndx >= d->d_size) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (NULL);
	}

	if (ec == ELFCLASS32) {

		move32 = (Elf32_Move *) d->d_buf + ndx;

		dst->m_value   = move32->m_value;
		dst->m_info    = (Elf64_Xword) move32->m_info;
		dst->m_poffset = (Elf64_Xword) move32->m_poffset;
		dst->m_repeat  = move32->m_repeat;
		dst->m_stride = move32->m_stride;
	} else {

		move64 = (Elf64_Move *) d->d_buf + ndx;

		*dst = *move64;
	}

	return (dst);
}

int
gelf_update_move(Elf_Data *d, int ndx, GElf_Move *gm)
{
	int ec;
	Elf *e;
	Elf_Scn *scn;
	Elf32_Move *move32;
	Elf64_Move *move64;
	size_t msz;
	uint32_t sh_type;

	if (d == NULL || ndx < 0 || gm == NULL ||
	    (scn = d->d_scn) == NULL ||
	    (e = scn->s_elf) == NULL) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	ec = e->e_class;
	assert(ec == ELFCLASS32 || ec == ELFCLASS64);

	if (ec == ELFCLASS32)
		sh_type = scn->s_shdr.s_shdr32.sh_type;
	else
		sh_type = scn->s_shdr.s_shdr64.sh_type;

	if (_libelf_xlate_shtype(sh_type) != ELF_T_MOVE) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	msz = _libelf_msize(ELF_T_MOVE, ec, e->e_version);
	assert(msz > 0);

	if (msz * ndx >= d->d_size) {
		LIBELF_SET_ERROR(ARGUMENT, 0);
		return (0);
	}

	if (ec == ELFCLASS32) {
		move32 = (Elf32_Move *) d->d_buf + ndx;

		move32->m_value  = gm->m_value;
		LIBELF_COPY_U32(move32, gm, m_info);
		LIBELF_COPY_U32(move32, gm, m_poffset);
		move32->m_repeat  = gm->m_repeat;
		move32->m_stride = gm->m_stride;

	} else {
		move64 = (Elf64_Move *) d->d_buf + ndx;

		*move64 = *gm;
	}

	return (1);
}

#endif	/* __FreeBSD_version >= 700025 */