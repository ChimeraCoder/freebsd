
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
#include <sys/time.h>
#include <sys/stdint.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ufs/dir.h>
#include <ufs/ffs/fs.h>

#include <err.h>
#include <string.h>

#include "fsck.h"

/*
 * Scan each entry in an ea block.
 */
int
eascan(struct inodesc *idesc, struct ufs2_dinode *dp)
{
#if 1
	return (0);
#else
	struct bufarea *bp;
	u_int dsize, n;
	u_char *cp;
	long blksiz;
	char dbuf[DIRBLKSIZ];

	printf("Inode %ju extsize %ju\n",
	   (intmax_t)idesc->id_number, (intmax_t)dp->di_extsize);
	if (dp->di_extsize == 0)
		return 0;
	if (dp->di_extsize <= sblock.fs_fsize)
		blksiz = sblock.fs_fsize;
	else
		blksiz = sblock.fs_bsize;
	printf("blksiz = %ju\n", (intmax_t)blksiz);
	bp = getdatablk(dp->di_extb[0], blksiz, BT_EXTATTR);
	cp = (u_char *)bp->b_un.b_buf;
	for (n = 0; n < blksiz; n++) {
		printf("%02x", cp[n]);
		if ((n & 31) == 31)
			printf("\n");
	}
	return (STOP);
#endif
}