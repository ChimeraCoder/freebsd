
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

/*
 * Internal APIs
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/errno.h>

#include <assert.h>
#include <libelf.h>
#include <stdlib.h>
#include <string.h>

#include "_libelf.h"

Elf *
_libelf_allocate_elf(void)
{
	Elf *e;

	if ((e = malloc(sizeof(*e))) == NULL) {
		LIBELF_SET_ERROR(RESOURCE, errno);
		return NULL;
	}

	e->e_activations = 1;
	e->e_arhdr	 = NULL;
	e->e_byteorder   = ELFDATANONE;
	e->e_class       = ELFCLASSNONE;
	e->e_cmd         = ELF_C_NULL;
	e->e_fd          = -1;
	e->e_flags	 = 0;
	e->e_kind        = ELF_K_NONE;
	e->e_parent      = NULL;
	e->e_rawfile     = NULL;
	e->e_rawsize     = 0;
	e->e_version     = LIBELF_PRIVATE(version);

	(void) memset(&e->e_u, 0, sizeof(e->e_u));

	return (e);
}

void
_libelf_init_elf(Elf *e, Elf_Kind kind)
{
	assert(e != NULL);
	assert(e->e_kind == ELF_K_NONE);

	e->e_kind = kind;

	switch (kind) {
	case ELF_K_ELF:
		STAILQ_INIT(&e->e_u.e_elf.e_scn);
		break;
	default:
		break;
	}
}

#define	FREE(P)		do {				\
		if (P)					\
			free(P);			\
	} while (0)


Elf *
_libelf_release_elf(Elf *e)
{
	switch (e->e_kind) {
	case ELF_K_AR:
		FREE(e->e_u.e_ar.e_symtab);
		break;

	case ELF_K_ELF:
		switch (e->e_class) {
		case ELFCLASS32:
			FREE(e->e_u.e_elf.e_ehdr.e_ehdr32);
			FREE(e->e_u.e_elf.e_phdr.e_phdr32);
			break;
		case ELFCLASS64:
			FREE(e->e_u.e_elf.e_ehdr.e_ehdr64);
			FREE(e->e_u.e_elf.e_phdr.e_phdr64);
			break;
		}

		assert(STAILQ_EMPTY(&e->e_u.e_elf.e_scn));

		if (e->e_arhdr) {
			FREE(e->e_arhdr->ar_name);
			FREE(e->e_arhdr->ar_rawname);
			free(e->e_arhdr);
		}

		break;

	default:
		break;
	}

	free(e);

	return (NULL);
}

Elf_Data *
_libelf_allocate_data(Elf_Scn *s)
{
	Elf_Data *d;

	if ((d = calloc((size_t) 1, sizeof(Elf_Data))) == NULL) {
		LIBELF_SET_ERROR(RESOURCE, 0);
		return (NULL);
	}

	d->d_scn = s;

	return (d);
}

Elf_Data *
_libelf_release_data(Elf_Data *d)
{

	if (d->d_flags & LIBELF_F_MALLOCED)
		free(d->d_buf);

	free(d);

	return (NULL);
}

Elf_Scn *
_libelf_allocate_scn(Elf *e, size_t ndx)
{
	Elf_Scn *s;

	if ((s = calloc((size_t) 1, sizeof(Elf_Scn))) == NULL) {
		LIBELF_SET_ERROR(RESOURCE, errno);
		return (NULL);
	}

	s->s_elf = e;
	s->s_ndx = ndx;

	STAILQ_INIT(&s->s_data);
	STAILQ_INIT(&s->s_rawdata);

	STAILQ_INSERT_TAIL(&e->e_u.e_elf.e_scn, s, s_next);

	return (s);
}

Elf_Scn *
_libelf_release_scn(Elf_Scn *s)
{
	Elf *e;
	Elf_Data *d, *td;

	assert(s != NULL);

	STAILQ_FOREACH_SAFE(d, &s->s_data, d_next, td) {
		STAILQ_REMOVE(&s->s_data, d, _Elf_Data, d_next);
		d = _libelf_release_data(d);
	}

	STAILQ_FOREACH_SAFE(d, &s->s_rawdata, d_next, td) {
		assert((d->d_flags & LIBELF_F_MALLOCED) == 0);
		STAILQ_REMOVE(&s->s_rawdata, d, _Elf_Data, d_next);
		d = _libelf_release_data(d);
	}

	e = s->s_elf;

	assert(e != NULL);

	STAILQ_REMOVE(&e->e_u.e_elf.e_scn, s, _Elf_Scn, s_next);

	free(s);

	return (NULL);
}