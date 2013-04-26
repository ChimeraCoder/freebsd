
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

/*
 * This elf_hash function is defined by the System V ABI.  It must be
 * kept compatible with "src/libexec/rtld-elf/rtld.c".
 */

unsigned long
elf_hash(const char *name)
{
	unsigned long h, t;
	const unsigned char *s;

	s = (const unsigned char *) name;
	h = t = 0;

	for (; *s != '\0'; h = h & ~t) {
		h = (h << 4) + *s++;
		t = h & 0xF0000000UL;
		if (t)
			h ^= t >> 24;
	}

	return (h);
}