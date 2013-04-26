
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

#include <bluetooth.h>
#include <sdp.h>
#include <uuid.h>
#include "uuid-private.h"

uint128_t	uuid_base = {
	.b = {
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00,
		0x10, 0x00,
		0x80, 0x00,
		0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb
	}
};

uint128_t	uuid_public_browse_group = {
	.b  = {
		0x00, 0x00, 0x10, 0x02,
		0x00, 0x00,
		0x10, 0x00,
		0x80, 0x00,
		0x00, 0x80, 0x5f, 0x9b, 0x34, 0xfb
	}
};