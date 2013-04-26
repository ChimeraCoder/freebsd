
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
static const char sccsid[] = "@(#)pass3.c	8.2 (Berkeley) 4/27/95";
#endif /* not lint */
#endif
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ufs/dir.h>
#include <ufs/ffs/fs.h>

#include <string.h>

#include "fsck.h"

void
pass3(void)
{
	struct inoinfo *inp;
	int loopcnt, inpindex, state;
	ino_t orphan;
	struct inodesc idesc;
	char namebuf[MAXNAMLEN+1];

	for (inpindex = inplast - 1; inpindex >= 0; inpindex--) {
		if (got_siginfo) {
			printf("%s: phase 3: dir %d of %d (%d%%)\n", cdevname,
			    (int)(inplast - inpindex - 1), (int)inplast,
			    (int)((inplast - inpindex - 1) * 100 / inplast));
			got_siginfo = 0;
		}
		if (got_sigalarm) {
			setproctitle("%s p3 %d%%", cdevname,
			    (int)((inplast - inpindex - 1) * 100 / inplast));
			got_sigalarm = 0;
		}
		inp = inpsort[inpindex];
		state = inoinfo(inp->i_number)->ino_state;
		if (inp->i_number == ROOTINO ||
		    (inp->i_parent != 0 && !S_IS_DUNFOUND(state)))
			continue;
		if (state == DCLEAR)
			continue;
		/*
		 * If we are running with soft updates and we come
		 * across unreferenced directories, we just leave
		 * them in DSTATE which will cause them to be pitched
		 * in pass 4.
		 */
		if ((preen || bkgrdflag) &&
		    resolved && usedsoftdep && S_IS_DUNFOUND(state)) {
			if (inp->i_dotdot >= ROOTINO)
				inoinfo(inp->i_dotdot)->ino_linkcnt++;
			continue;
		}
		for (loopcnt = 0; ; loopcnt++) {
			orphan = inp->i_number;
			if (inp->i_parent == 0 ||
			    !INO_IS_DUNFOUND(inp->i_parent) ||
			    loopcnt > countdirs)
				break;
			inp = getinoinfo(inp->i_parent);
		}
		if (loopcnt <= countdirs) {
			if (linkup(orphan, inp->i_dotdot, NULL)) {
				inp->i_parent = inp->i_dotdot = lfdir;
				inoinfo(lfdir)->ino_linkcnt--;
			}
			inoinfo(orphan)->ino_state = DFOUND;
			propagate();
			continue;
		}
		pfatal("ORPHANED DIRECTORY LOOP DETECTED I=%lu",
		    (u_long)orphan);
		if (reply("RECONNECT") == 0)
			continue;
		memset(&idesc, 0, sizeof(struct inodesc));
		idesc.id_type = DATA;
		idesc.id_number = inp->i_parent;
		idesc.id_parent = orphan;
		idesc.id_func = findname;
		idesc.id_name = namebuf;
		if ((ckinode(ginode(inp->i_parent), &idesc) & FOUND) == 0)
			pfatal("COULD NOT FIND NAME IN PARENT DIRECTORY");
		if (linkup(orphan, inp->i_parent, namebuf)) {
			idesc.id_func = clearentry;
			if (ckinode(ginode(inp->i_parent), &idesc) & FOUND)
				inoinfo(orphan)->ino_linkcnt++;
			inp->i_parent = inp->i_dotdot = lfdir;
			inoinfo(lfdir)->ino_linkcnt--;
		}
		inoinfo(orphan)->ino_state = DFOUND;
		propagate();
	}
}