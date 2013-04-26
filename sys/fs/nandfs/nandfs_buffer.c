
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
#include <sys/conf.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/mutex.h>
#include <sys/buf.h>
#include <sys/namei.h>
#include <sys/vnode.h>
#include <sys/bio.h>

#include <fs/nandfs/nandfs_mount.h>
#include <fs/nandfs/nandfs.h>
#include <fs/nandfs/nandfs_subr.h>

struct buf *
nandfs_geteblk(int size, int flags)
{
	struct buf *bp;

	/*
	 * XXX
	 * Right now we can call geteblk with GB_NOWAIT_BD flag, which means
	 * it can return NULL. But we cannot afford to get NULL, hence this panic.
	 */
	bp = geteblk(size, flags);
	if (bp == NULL)
		panic("geteblk returned NULL");

	return (bp);
}

void
nandfs_dirty_bufs_increment(struct nandfs_device *fsdev)
{

	mtx_lock(&fsdev->nd_mutex);
	KASSERT(fsdev->nd_dirty_bufs >= 0, ("negative nd_dirty_bufs"));
	fsdev->nd_dirty_bufs++;
	mtx_unlock(&fsdev->nd_mutex);
}

void
nandfs_dirty_bufs_decrement(struct nandfs_device *fsdev)
{

	mtx_lock(&fsdev->nd_mutex);
	KASSERT(fsdev->nd_dirty_bufs > 0,
	    ("decrementing not-positive nd_dirty_bufs"));
	fsdev->nd_dirty_bufs--;
	mtx_unlock(&fsdev->nd_mutex);
}