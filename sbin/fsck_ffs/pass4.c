
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

#if 0
#ifndef lint
static const char sccsid[] = "@(#)pass4.c	8.4 (Berkeley) 4/28/95";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>

#include <err.h>
#include <stdint.h>
#include <string.h>

#include "fsck.h"

void
pass4(void)
{
	ino_t inumber;
	union dinode *dp;
	struct inodesc idesc;
	int i, n, cg;

	memset(&idesc, 0, sizeof(struct inodesc));
	idesc.id_type = ADDR;
	idesc.id_func = pass4check;
	for (cg = 0; cg < sblock.fs_ncg; cg++) {
		if (got_siginfo) {
			printf("%s: phase 4: cyl group %d of %d (%d%%)\n",
			    cdevname, cg, sblock.fs_ncg,
			    cg * 100 / sblock.fs_ncg);
			got_siginfo = 0;
		}
		if (got_sigalarm) {
			setproctitle("%s p4 %d%%", cdevname,
			    cg * 100 / sblock.fs_ncg);
			got_sigalarm = 0;
		}
		inumber = cg * sblock.fs_ipg;
		for (i = 0; i < inostathead[cg].il_numalloced; i++, inumber++) {
			if (inumber < ROOTINO)
				continue;
			idesc.id_number = inumber;
			switch (inoinfo(inumber)->ino_state) {

			case FZLINK:
			case DZLINK:
				if (inoinfo(inumber)->ino_linkcnt == 0) {
					clri(&idesc, "UNREF", 1);
					break;
				}
				/* fall through */

			case FSTATE:
			case DFOUND:
				n = inoinfo(inumber)->ino_linkcnt;
				if (n) {
					adjust(&idesc, (short)n);
					break;
				}
				break;

			case DSTATE:
				clri(&idesc, "UNREF", 1);
				break;

			case DCLEAR:
				/* if on snapshot, already cleared */
				if (cursnapshot != 0)
					break;
				dp = ginode(inumber);
				if (DIP(dp, di_size) == 0) {
					clri(&idesc, "ZERO LENGTH", 1);
					break;
				}
				/* fall through */
			case FCLEAR:
				clri(&idesc, "BAD/DUP", 1);
				break;

			case USTATE:
				break;

			default:
				errx(EEXIT, "BAD STATE %d FOR INODE I=%ju",
				    inoinfo(inumber)->ino_state,
				    (uintmax_t)inumber);
			}
		}
	}
}

int
pass4check(struct inodesc *idesc)
{
	struct dups *dlp;
	int nfrags, res = KEEPON;
	ufs2_daddr_t blkno = idesc->id_blkno;

	for (nfrags = idesc->id_numfrags; nfrags > 0; blkno++, nfrags--) {
		if (chkrange(blkno, 1)) {
			res = SKIP;
		} else if (testbmap(blkno)) {
			for (dlp = duplist; dlp; dlp = dlp->next) {
				if (dlp->dup != blkno)
					continue;
				dlp->dup = duplist->dup;
				dlp = duplist;
				duplist = duplist->next;
				free((char *)dlp);
				break;
			}
			if (dlp == 0) {
				clrbmap(blkno);
				n_blks--;
			}
		}
	}
	return (res);
}