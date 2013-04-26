
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

#ifdef lint
static const char sccsid[] = "@(#)mbufs.c	8.1 (Berkeley) 6/6/93";
#endif

#include <sys/param.h>
#include <sys/types.h>
#include <sys/mbuf.h>
#include <sys/sysctl.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <paths.h>

#include "systat.h"
#include "extern.h"

static struct mbstat *mbstat;
static long *m_mbtypes;
static short nmbtypes;

static struct mtnames {
	short mt_type;
	const char *mt_name;
} mtnames[] = {
	{ MT_DATA, 	"data"},
	{ MT_HEADER,	"headers"},
	{ MT_SONAME,	"socknames"},
	{ MT_CONTROL,	"control"},
	{ MT_OOBDATA,	"oobdata"}
};
#define	NNAMES	(sizeof (mtnames) / sizeof (mtnames[0]))

WINDOW *
openmbufs(void)
{
	return (subwin(stdscr, LINES-3-1, 0, MAINWIN_ROW, 0));
}

void
closembufs(WINDOW *w)
{
	if (w == NULL)
		return;
	wclear(w);
	wrefresh(w);
	delwin(w);
}

void
labelmbufs(void)
{
	wmove(wnd, 0, 0); wclrtoeol(wnd);
	mvwaddstr(wnd, 0, 10,
	    "/0   /5   /10  /15  /20  /25  /30  /35  /40  /45  /50  /55  /60");
}

void
showmbufs(void)
{
	int i, j, max, idx;
	u_long totmbufs;
	char buf[10];
	const char *mtname;

	totmbufs = mbstat->m_mbufs;

	/*
	 * Print totals for different mbuf types.
	 */
	for (j = 0; j < wnd->_maxy; j++) {
		max = 0, idx = -1;
		for (i = 0; i < wnd->_maxy; i++) {
			if (i == MT_NOTMBUF)
				continue;
			if (i >= nmbtypes)
				break;
			if (m_mbtypes[i] > max) {
				max = m_mbtypes[i];
				idx = i;
			}
		}
		if (max == 0)
			break;

		mtname = NULL;
		for (i = 0; i < (int)NNAMES; i++)
			if (mtnames[i].mt_type == idx)
				mtname = mtnames[i].mt_name;
		if (mtname == NULL)
			mvwprintw(wnd, 1+j, 0, "%10d", idx);
		else
			mvwprintw(wnd, 1+j, 0, "%-10.10s", mtname);
		wmove(wnd, 1 + j, 10);
		if (max > 60) {
			snprintf(buf, sizeof(buf), " %d", max);
			max = 60;
			while (max--)
				waddch(wnd, 'X');
			waddstr(wnd, buf);
		} else
			while (max--)
				waddch(wnd, 'X');
		wclrtoeol(wnd);
		m_mbtypes[idx] = 0;
	}

	/*
	 * Print total number of free mbufs.
	 */
	if (totmbufs > 0) {
		mvwprintw(wnd, 1+j, 0, "%-10.10s", "Mbufs");
		if (totmbufs > 60) {
			snprintf(buf, sizeof(buf), " %lu", totmbufs);
			totmbufs = 60;
			while(totmbufs--)
				waddch(wnd, 'X');
			waddstr(wnd, buf);
		} else {
			while(totmbufs--)
				waddch(wnd, 'X');
		}
		wclrtoeol(wnd);
		j++;
	}
	wmove(wnd, 1+j, 0); wclrtobot(wnd);
}

int
initmbufs(void)
{
	size_t len;

	len = sizeof *mbstat;
	if ((mbstat = malloc(len)) == NULL) {
		error("malloc mbstat failed");
		return 0;
	}
	if (sysctlbyname("kern.ipc.mbstat", mbstat, &len, NULL, 0) < 0) {
		error("sysctl retrieving mbstat");
		return 0;
	}
	nmbtypes = mbstat->m_numtypes;
	if ((m_mbtypes = calloc(nmbtypes, sizeof(long *))) == NULL) {
		error("calloc m_mbtypes failed");
		return 0;
	}

	return 1;
}

void
fetchmbufs(void)
{
	size_t len;

	len = sizeof *mbstat;
	if (sysctlbyname("kern.ipc.mbstat", mbstat, &len, NULL, 0) < 0)
		printw("sysctl: mbstat: %s", strerror(errno));
}