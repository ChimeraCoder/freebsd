
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

#include <machine/elf.h>
#include <machine/endian.h>

#include <libelf.h>

#include "_libelf.h"

struct _libelf_globals _libelf = {
	.libelf_arch		= ELF_ARCH,
	.libelf_byteorder	= ELF_TARG_DATA,
	.libelf_class		= ELF_TARG_CLASS,
	.libelf_error		= 0,
	.libelf_fillchar	= 0,
	.libelf_version		= EV_NONE
};


#if defined(LIBELF_TEST_HOOKS)
int
_libelf_get_elf_class(Elf *elf)
{
	return elf->e_class;
}

void
_libelf_set_elf_class(Elf *elf, int c)
{
	elf->e_class = c;
}
#endif	/* LIBELF_TEST_HOOKS */