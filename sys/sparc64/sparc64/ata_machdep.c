
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
#include <geom/geom_disk.h>
#include <machine/md_var.h>

void
sparc64_ata_disk_firmware_geom_adjust(struct disk *disk)
{

	/*
	 * The VTOC8 disk label only uses 16-bit fields for cylinders, heads
	 * and sectors so the geometry of large disks has to be adjusted.
	 * If the disk is > 32GB at 16 heads and 63 sectors, adjust to 255
	 * sectors (this matches what the OpenSolaris dad(7D) driver does).
	 * If the disk is even > 128GB, additionally adjust the heads to
	 * 255.  This allows disks up to the 2TB limit of the extended VTOC8.
	 * XXX the OpenSolaris dad(7D) driver limits the mediasize to 128GB.
	 */
	if (disk->d_mediasize > (off_t)65535 * 16 * 63 * disk->d_sectorsize)
		disk->d_fwsectors = 255;
	if (disk->d_mediasize > (off_t)65535 * 16 * 255 * disk->d_sectorsize)
		disk->d_fwheads = 255;
}