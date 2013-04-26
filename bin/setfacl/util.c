
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

#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "setfacl.h"

void *
zmalloc(size_t size)
{
	void *ptr;

	ptr = calloc(1, size);
	if (ptr == NULL)
		err(1, "calloc() failed");
	return (ptr);
}

const char *
brand_name(int brand)
{
	switch (brand) {
	case ACL_BRAND_NFS4:
		return "NFSv4";
	case ACL_BRAND_POSIX:
		return "POSIX.1e";
	default:
		return "unknown";
	}
}

int
branding_mismatch(int brand1, int brand2)
{
	if (brand1 == ACL_BRAND_UNKNOWN || brand2 == ACL_BRAND_UNKNOWN)
		return (0);
	if (brand1 != brand2)
		return (1);
	return (0);
}