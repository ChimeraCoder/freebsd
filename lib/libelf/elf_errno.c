
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

#include <libelf.h>

#include "_libelf.h"

int
elf_errno(void)
{
	int old;

	old = LIBELF_PRIVATE(error);
	LIBELF_PRIVATE(error) = 0;
	return (old & LIBELF_ELF_ERROR_MASK);
}

#if	defined(LIBELF_TEST_HOOKS)

int
_libelf_get_max_error(void)
{
	return ELF_E_NUM;
}

void
_libelf_set_error(int error)
{
	LIBELF_PRIVATE(error) = error;
}

#endif	/* LIBELF_TEST_HOOKS */