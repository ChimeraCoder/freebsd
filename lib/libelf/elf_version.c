
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

unsigned int
elf_version(unsigned int v)
{
	unsigned int old;

	if ((old = LIBELF_PRIVATE(version)) == EV_NONE)
		old = EV_CURRENT;

	if (v == EV_NONE)
		return old;
	if (v > EV_CURRENT) {
		LIBELF_SET_ERROR(VERSION, 0);
		return EV_NONE;
	}

	LIBELF_PRIVATE(version) = v;
	return (old);
}