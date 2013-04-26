
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
#include <sys/vnode.h>
#include <sys/mount.h>

#include <fs/cd9660/iso.h>
#include <fs/cd9660/cd9660_node.h>

/*
 * Bmap converts the logical block number of a file to its physical
 * block number on the disk. The conversion is done by using the
 * logical block number to index into the data block (extent) for the
 * file.
 */
int
cd9660_bmap(ap)
	struct vop_bmap_args /* {
		struct vnode *a_vp;
		daddr_t  a_bn;
		struct bufobj **a_bop;
		daddr_t *a_bnp;
		int *a_runp;
		int *a_runb;
	} */ *ap;
{
	struct iso_node *ip = VTOI(ap->a_vp);
	daddr_t lblkno = ap->a_bn;
	int bshift;

	/*
	 * Check for underlying vnode requests and ensure that logical
	 * to physical mapping is requested.
	 */
	if (ap->a_bop != NULL)
		*ap->a_bop = &ip->i_mnt->im_devvp->v_bufobj;
	if (ap->a_bnp == NULL)
		return (0);

	/*
	 * Compute the requested block number
	 */
	bshift = ip->i_mnt->im_bshift;
	*ap->a_bnp = (ip->iso_start + lblkno) << (bshift - DEV_BSHIFT);

	/*
	 * Determine maximum number of readahead blocks following the
	 * requested block.
	 */
	if (ap->a_runp) {
		int nblk;

		nblk = (ip->i_size >> bshift) - (lblkno + 1);
		if (nblk <= 0)
			*ap->a_runp = 0;
		else if (nblk >= (MAXBSIZE >> bshift))
			*ap->a_runp = (MAXBSIZE >> bshift) - 1;
		else
			*ap->a_runp = nblk;
	}

	if (ap->a_runb) {
		*ap->a_runb = 0;
	}

	return 0;
}