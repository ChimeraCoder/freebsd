
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

#include <sys/types.h>
#include <sys/sysctl.h>
#include <string.h>

#include "partedit.h"

static char platform[255] = "";

const char *
default_scheme(void) {
	size_t platlen = sizeof(platform);
	if (strlen(platform) == 0)
		sysctlbyname("hw.platform", platform, &platlen, NULL, -1);

	if (strcmp(platform, "powermac") == 0)
		return ("APM");
	if (strcmp(platform, "chrp") == 0)
		return ("MBR");

	/* Pick GPT (bootable on PS3) as a generic default */
	return ("GPT");
}

int
is_scheme_bootable(const char *part_type) {
	size_t platlen = sizeof(platform);
	if (strlen(platform) == 0)
		sysctlbyname("hw.platform", platform, &platlen, NULL, -1);

	if (strcmp(platform, "powermac") == 0 && strcmp(part_type, "APM") == 0)
		return (1);
	if (strcmp(platform, "ps3") == 0 && strcmp(part_type, "GPT") == 0)
		return (1);
	if (strcmp(platform, "chrp") == 0 && strcmp(part_type, "MBR") == 0)
		return (1);

	return (0);
}

size_t
bootpart_size(const char *part_type) {
	if (strcmp(part_type, "APM") == 0 || strcmp(part_type, "MBR") == 0)
		return (800*1024);
	return (0);
}

const char *
bootcode_path(const char *part_type) {
	return (NULL);
}
	
const char *
partcode_path(const char *part_type) {
	if (strcmp(part_type, "APM") == 0)
		return ("/boot/boot1.hfs");
	if (strcmp(part_type, "MBR") == 0)
		return ("/boot/boot1.elf");
	return (NULL);
}