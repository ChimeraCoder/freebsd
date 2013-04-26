
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
#include <sys/systm.h>

#include <cam/cam.h>
#include <cam/cam_ccb.h>

#include <machine/md_var.h>

int
scsi_da_bios_params(struct ccb_calc_geometry *ccg)
{
	uint32_t secs_per_cylinder, size_mb;

	/*
	 * The VTOC8 disk label only uses 16-bit fields for cylinders, heads
	 * and sectors so the geometry of large disks has to be adjusted.
	 * We generally use the sizing used by cam_calc_geometry(9), except
	 * when it would overflow the cylinders, in which case we use 255
	 * heads and sectors.  This allows disks up to the 2TB limit of the
	 * extended VTOC8.
	 * XXX this doesn't match the sizing used by OpenSolaris, as that
	 * would exceed the 8-bit ccg->heads and ccg->secs_per_track.
	 */
	if (ccg->block_size == 0)
		return (0);
	size_mb = (1024L * 1024L) / ccg->block_size;
	if (size_mb == 0)
		return (0);
	size_mb = ccg->volume_size / size_mb;
	if (ccg->volume_size > (uint64_t)65535 * 255 * 63) {
		ccg->heads = 255;
		ccg->secs_per_track = 255;
	} else if (size_mb > 1024) {
		ccg->heads = 255;
		ccg->secs_per_track = 63;
	} else {
		ccg->heads = 64;
		ccg->secs_per_track = 32;
	}
	secs_per_cylinder = ccg->heads * ccg->secs_per_track;
	if (secs_per_cylinder == 0)
		return (0);
	ccg->cylinders = ccg->volume_size / secs_per_cylinder;
	return (1);
}