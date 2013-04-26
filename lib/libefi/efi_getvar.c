
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
 * GetVariable(
 *	IN CHAR16	*VariableName,
 *	IN EFI_GUID	*VendorGuid,
 *	OUT UINT32	*Attributes OPTIONAL,
 *	IN OUT UINTN	*DataSize,
 *	OUT VOID	*Data
 *   );
 */

int
efi_getvar(char *name, uuid_t *vendor, uint32_t *attrib, size_t *datasize,
    void *data)
{
	struct iodev_efivar_req req;
	int error;

	req.namesize = 0;
	error = libefi_utf8_to_ucs2(name, &req.namesize, &req.name);
	if (error)
		return (error);

	req.vendor = *vendor;
	req.datasize = *datasize;
	req.data = data;
	req.access = IODEV_EFIVAR_GETVAR;
	error = libefi_efivar(&req);
	*datasize = req.datasize;
	if (!error && attrib != NULL)
		*attrib = req.attrib;
	free(req.name);
	return (error);
}