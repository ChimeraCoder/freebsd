
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
	/*
	 * Our loader can parse GPT, so pick that as the default for lack of
	 * a better idea.
	 */

	return ("GPT");
}

int
is_scheme_bootable(const char *part_type) {
	/*
	 * We don't know anything about this platform, so don't irritate the
	 * user by claiming the chosen partition scheme isn't bootable.
	 */

	return (1);
}

/* No clue => no boot partition, bootcode, or partcode */

size_t
bootpart_size(const char *part_type) {
	return (0);
}

const char *
bootcode_path(const char *part_type) {
	return (NULL);
}
	
const char *
partcode_path(const char *part_type) {
	return (NULL);
}