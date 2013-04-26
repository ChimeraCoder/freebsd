
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

#include <efi.h>
#include <efilib.h>

int
efi_status_to_errno(EFI_STATUS status)
{
	int errno;

	switch (status) {
	case EFI_ACCESS_DENIED:
		errno = EPERM;
		break;

	case EFI_BUFFER_TOO_SMALL:
		errno = EOVERFLOW;
		break;

	case EFI_DEVICE_ERROR:
	case EFI_VOLUME_CORRUPTED:
		errno = EIO;
		break;

	case EFI_INVALID_PARAMETER:
		errno = EINVAL;
		break;

	case EFI_MEDIA_CHANGED:
		errno = ESTALE;
		break;

	case EFI_NO_MEDIA:
		errno = ENXIO;
		break;

	case EFI_NOT_FOUND:
		errno = ENOENT;
		break;

	case EFI_OUT_OF_RESOURCES:
		errno = ENOMEM;
		break;

	case EFI_UNSUPPORTED:
		errno = ENODEV;
		break;

	case EFI_VOLUME_FULL:
		errno = ENOSPC;
		break;

	case EFI_WRITE_PROTECTED:
		errno = EACCES;
		break;

	case 0:
		errno = 0;
		break;

	default:
		errno = EDOOFUS;
		break;
	}

	return (errno);
}