
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
#include <sys/linker.h>
#include <err.h>
#include <string.h>

#include "asf.h"

/*
 * Get the linker file list using the kld interface.
 * Works with a live kernel only.
 */
void
asf_kld(void)
{
	struct kld_file_stat kfs;
	int fid = 0;	/* indicates the beginning of the linker file list */

	while ((fid = kldnext(fid)) != 0) {
		if (fid == -1)
			err(2, "kldnext");
		kfs.version = sizeof(kfs);	/* must be set for kldstat(2) */
		/* Get info on this linker file */
		if (kldstat(fid, &kfs) == -1)
			err(2, "kldstat");
		if (strcmp(kfs.name, KERNFILE) == 0)
			continue;
		/* Add to our list of linker files */
		kfile_add(kfs.name, kfs.address);
	}
}