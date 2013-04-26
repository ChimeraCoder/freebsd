
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
static char sccsid[] = "@(#)pigs.c	8.2 (Berkeley) 9/23/93";
#endif /* not lint */
#endif

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

/*
 * Pigs display from Bill Reeves at Lucasfilm
 */

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/user.h>

#include <curses.h>
#include <math.h>
#include <pwd.h>
#include <stdlib.h>

#include "systat.h"
#include "extern.h"

int compar(const void *, const void *);

static int nproc;
static struct p_times {
	float pt_pctcpu;
	struct kinfo_proc *pt_kp;
} *pt;

static int    fscale;
static double  lccpu;

WINDOW *
openpigs(void)
{
	return (subwin(stdscr, LINES-3-1, 0, MAINWIN_ROW, 0));
}

void
closepigs(WINDOW *w)
{
	if (w == NULL)
		return;
	wclear(w);
	wrefresh(w);
	delwin(w);
}

void
showpigs(void)
{
	int i, j, y, k;
	const char *uname, *pname;
	char pidname[30];

	if (pt == NULL)
		return;

	qsort(pt, nproc, sizeof (struct p_times), compar);
	y = 1;
	i = nproc;
	if (i > wnd->_maxy-1)
		i = wnd->_maxy-1;
	for (k = 0; i > 0 && pt[k].pt_pctcpu > 0.01; i--, y++, k++) {
		uname = user_from_uid(pt[k].pt_kp->ki_uid, 0);
		pname = pt[k].pt_kp->ki_comm;
		wmove(wnd, y, 0);
		wclrtoeol(wnd);
		mvwaddstr(wnd, y, 0, uname);
		snprintf(pidname, sizeof(pidname), "%10.10s", pname);
		mvwaddstr(wnd, y, 9, pidname);
		wmove(wnd, y, 20);
		for (j = pt[k].pt_pctcpu * 50 + 0.5; j > 0; j--)
			waddch(wnd, 'X');
	}
	wmove(wnd, y, 0); wclrtobot(wnd);
}

int
initpigs(void)
{
	fixpt_t ccpu;
	size_t len;
	int err;

	len = sizeof(ccpu);
	err = sysctlbyname("kern.ccpu", &ccpu, &len, NULL, 0);
	if (err || len != sizeof(ccpu)) {
		perror("kern.ccpu");
		return (0);
	}

	len = sizeof(fscale);
	err = sysctlbyname("kern.fscale", &fscale, &len, NULL, 0);
	if (err || len != sizeof(fscale)) {
		perror("kern.fscale");
		return (0);
	}

	lccpu = log((double) ccpu / fscale);

	return(1);
}

void
fetchpigs(void)
{
	int i;
	float ftime;
	float *pctp;
	struct kinfo_proc *kpp;
	static int lastnproc = 0;

	if ((kpp = kvm_getprocs(kd, KERN_PROC_ALL, 0, &nproc)) == NULL) {
		error("%s", kvm_geterr(kd));
		if (pt)
			free(pt);
		return;
	}
	if (nproc > lastnproc) {
		free(pt);
		if ((pt =
		    malloc(nproc * sizeof(struct p_times))) == NULL) {
			error("Out of memory");
			die(0);
		}
	}
	lastnproc = nproc;
	/*
	 * calculate %cpu for each proc
	 */
	for (i = 0; i < nproc; i++) {
		pt[i].pt_kp = &kpp[i];
		pctp = &pt[i].pt_pctcpu;
		ftime = kpp[i].ki_swtime;
		if (ftime == 0 || (kpp[i].ki_flag & P_INMEM) == 0)
			*pctp = 0;
		else
			*pctp = ((double) kpp[i].ki_pctcpu /
					fscale) / (1.0 - exp(ftime * lccpu));
	}
}

void
labelpigs(void)
{
	wmove(wnd, 0, 0);
	wclrtoeol(wnd);
	mvwaddstr(wnd, 0, 20,
	    "/0%  /10  /20  /30  /40  /50  /60  /70  /80  /90  /100");
}

int
compar(const void *a, const void *b)
{
	return (((const struct p_times *) a)->pt_pctcpu >
		((const struct p_times *) b)->pt_pctcpu)? -1: 1;
}