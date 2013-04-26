
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
static char sccsid[] = "@(#)proc_compare.c	8.2 (Berkeley) 9/23/93";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/time.h>
#include <sys/user.h>

#include "extern.h"

/*
 * Returns 1 if p2 is "better" than p1
 *
 * The algorithm for picking the "interesting" process is thus:
 *
 *	1) Only foreground processes are eligible - implied.
 *	2) Runnable processes are favored over anything else.  The runner
 *	   with the highest cpu utilization is picked (p_estcpu).  Ties are
 *	   broken by picking the highest pid.
 *	3) The sleeper with the shortest sleep time is next.  With ties,
 *	   we pick out just "short-term" sleepers (TDF_SINTR == 0).
 *	4) Further ties are broken by picking the highest pid.
 *
 * If you change this, be sure to consider making the change in the kernel
 * too (^T in kern/tty.c).
 *
 * TODO - consider whether pctcpu should be used.
 */

#define	ISRUN(p)	(((p)->ki_stat == SRUN) || ((p)->ki_stat == SIDL))
#define	TESTAB(a, b)    ((a)<<1 | (b))
#define	ONLYA   2
#define	ONLYB   1
#define	BOTH    3

int
proc_compare(struct kinfo_proc *p1, struct kinfo_proc *p2)
{

	if (p1 == NULL)
		return (1);
	/*
	 * see if at least one of them is runnable
	 */
	switch (TESTAB(ISRUN(p1), ISRUN(p2))) {
	case ONLYA:
		return (0);
	case ONLYB:
		return (1);
	case BOTH:
		/*
		 * tie - favor one with highest recent cpu utilization
		 */
		if (p2->ki_estcpu > p1->ki_estcpu)
			return (1);
		if (p1->ki_estcpu > p2->ki_estcpu)
			return (0);
		return (p2->ki_pid > p1->ki_pid); /* tie - return highest pid */
	}
	/*
	 * weed out zombies
	 */
	switch (TESTAB(p1->ki_stat == SZOMB, p2->ki_stat == SZOMB)) {
	case ONLYA:
		return (1);
	case ONLYB:
		return (0);
	case BOTH:
		return (p2->ki_pid > p1->ki_pid); /* tie - return highest pid */
	}
	/*
	 * pick the one with the smallest sleep time
	 */
	if (p2->ki_slptime > p1->ki_slptime)
		return (0);
	if (p1->ki_slptime > p2->ki_slptime)
		return (1);
	/*
	 * favor one sleeping in a non-interruptible sleep
	 */
	if (p1->ki_tdflags & TDF_SINTR && (p2->ki_tdflags & TDF_SINTR) == 0)
		return (1);
	if (p2->ki_tdflags & TDF_SINTR && (p1->ki_tdflags & TDF_SINTR) == 0)
		return (0);
	return (p2->ki_pid > p1->ki_pid);	/* tie - return highest pid */
}