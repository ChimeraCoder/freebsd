
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

#include <string.h>

#include "partedit.h"

const char *
default_scheme(void) {
	return ("GPT");
}

int
is_scheme_bootable(const char *part_type) {
	if (strcmp(part_type, "BSD") == 0)
		return (1);
	if (strcmp(part_type, "GPT") == 0)
		return (1);
	if (strcmp(part_type, "MBR") == 0)
		return (1);

	return (0);
}

size_t
bootpart_size(const char *part_type) {
	if (strcmp(part_type, "GPT") == 0)
		return (64*1024);

	/* No partcode except for GPT */
	return (0);
}

const char *
bootcode_path(const char *part_type) {
	if (strcmp(part_type, "GPT") == 0)
		return ("/boot/pmbr");
	if (strcmp(part_type, "MBR") == 0)
		return ("/boot/mbr");
	if (strcmp(part_type, "BSD") == 0)
		return ("/boot/boot");

	return (NULL);
}
	
const char *
partcode_path(const char *part_type) {
	if (strcmp(part_type, "GPT") == 0)
		return ("/boot/gptboot");
	
	/* No partcode except for GPT */
	return (NULL);
}