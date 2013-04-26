
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
#include "notes.h"

/*
 * Special ".note" entry specifying the ABI version.  See
 * http://www.netbsd.org/Documentation/kernel/elf-notes.html
 * for more information.
 *
 * For all arches except sparc, gcc emits the section directive for the
 * following struct with a PROGBITS type.  However, newer versions of binutils
 * (after 2.16.90) require the section to be of NOTE type, to guarantee that the
 * .note.ABI-tag section correctly ends up in the first page of the final
 * executable.
 *
 * Unfortunately, there is no clean way to tell gcc to use another section type,
 * so this C file (or the C file that includes it) must be compiled in multiple
 * steps:
 *
 * - Compile the .c file to a .s file.
 * - Edit the .s file to change the 'progbits' type to 'note', for the section
 *   directive that defines the .note.ABI-tag section.
 * - Compile the .s file to an object file.
 *
 * These steps are done in the invididual Makefiles for each applicable arch.
 */
static const struct {
	int32_t	namesz;
	int32_t	descsz;
	int32_t	type;
	char	name[sizeof(NOTE_FREEBSD_VENDOR)];
	int32_t	desc;
} abitag __attribute__ ((section (NOTE_SECTION), aligned(4))) __used = {
	.namesz = sizeof(NOTE_FREEBSD_VENDOR),
	.descsz = sizeof(int32_t),
	.type = ABI_NOTETYPE,
	.name = NOTE_FREEBSD_VENDOR,
	.desc = __FreeBSD_version
};