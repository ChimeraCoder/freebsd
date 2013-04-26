
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

#include <libefi.h>
#include <stdlib.h>

#include "libefi_int.h"

/*
 * EFI_STATUS
 * GetNextVariableName(
 *	IN OUT UINTN	*VariableNameSize,
 *	IN OUT CHAR16	*VariableName,
 *	IN OUT EFI_GUID	*VendorGuid
 *    );
 */

int
efi_nextvarname(size_t *namesize, char *name, uuid_t *vendor)
{
	struct iodev_efivar_req req;
	int error;

	req.namesize = *namesize;
	error = libefi_utf8_to_ucs2(name, &req.namesize, &req.name);
	if (error)
		return (error);

	req.vendor = *vendor;
	req.access = IODEV_EFIVAR_NEXTNAME;
	error = libefi_efivar(&req);
	*namesize = req.namesize;
	if (!error) {
		error = libefi_ucs2_to_utf8(req.name, namesize, name);
		if (!error)
			*vendor = req.vendor;
	}
	free(req.name);
	return (error);
}