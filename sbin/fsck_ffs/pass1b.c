
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
static const char sccsid[] = "@(#)pass1b.c	8.4 (Berkeley) 4/28/95";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>

#include <string.h>

#include "fsck.h"

static  struct dups *duphead;
static int pass1bcheck(struct inodesc *);

void
pass1b(void)
{
	int c, i;
	union dinode *dp;
	struct inodesc idesc;
	ino_t inumber;

	memset(&idesc, 0, sizeof(struct inodesc));
	idesc.id_type = ADDR;
	idesc.id_func = pass1bcheck;
	duphead = duplist;
	inumber = 0;
	for (c = 0; c < sblock.fs_ncg; c++) {
		if (got_siginfo) {
			printf("%s: phase 1b: cyl group %d of %d (%d%%)\n",
			    cdevname, c, sblock.fs_ncg,
			    c * 100 / sblock.fs_ncg);
			got_siginfo = 0;
		}
		if (got_sigalarm) {
			setproctitle("%s p1b %d%%", cdevname,
			    c * 100 / sblock.fs_ncg);
			got_sigalarm = 0;
		}
		for (i = 0; i < sblock.fs_ipg; i++, inumber++) {
			if (inumber < ROOTINO)
				continue;
			dp = ginode(inumber);
			if (dp == NULL)
				continue;
			idesc.id_number = inumber;
			if (inoinfo(inumber)->ino_state != USTATE &&
			    (ckinode(dp, &idesc) & STOP))
				return;
		}
	}
}

static int
pass1bcheck(struct inodesc *idesc)
{
	struct dups *dlp;
	int nfrags, res = KEEPON;
	ufs2_daddr_t blkno = idesc->id_blkno;

	for (nfrags = idesc->id_numfrags; nfrags > 0; blkno++, nfrags--) {
		if (chkrange(blkno, 1))
			res = SKIP;
		for (dlp = duphead; dlp; dlp = dlp->next) {
			if (dlp->dup == blkno) {
				blkerror(idesc->id_number, "DUP", blkno);
				dlp->dup = duphead->dup;
				duphead->dup = blkno;
				duphead = duphead->next;
			}
			if (dlp == muldup)
				break;
		}
		if (muldup == 0 || duphead == muldup->next)
			return (STOP);
	}
	return (res);
}