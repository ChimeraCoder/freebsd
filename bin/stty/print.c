
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

#ifndef lint
#if 0
static char sccsid[] = "@(#)print.c	8.6 (Berkeley) 4/16/94";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "stty.h"
#include "extern.h"

static void  binit(const char *);
static void  bput(const char *);
static const char *ccval(struct cchar *, int);

void
print(struct termios *tp, struct winsize *wp, int ldisc, enum FMT fmt)
{
	struct cchar *p;
	long tmp;
	u_char *cc;
	int cnt, ispeed, ospeed;
	char buf1[100], buf2[100];

	cnt = 0;

	/* Line discipline. */
	if (ldisc != TTYDISC) {
		switch(ldisc) {
		case SLIPDISC:
			cnt += printf("slip disc; ");
			break;
		case PPPDISC:
			cnt += printf("ppp disc; ");
			break;
		default:
			cnt += printf("#%d disc; ", ldisc);
			break;
		}
	}

	/* Line speed. */
	ispeed = cfgetispeed(tp);
	ospeed = cfgetospeed(tp);
	if (ispeed != ospeed)
		cnt +=
		    printf("ispeed %d baud; ospeed %d baud;", ispeed, ospeed);
	else
		cnt += printf("speed %d baud;", ispeed);
	if (fmt >= BSD)
		cnt += printf(" %d rows; %d columns;", wp->ws_row, wp->ws_col);
	if (cnt)
		(void)printf("\n");

#define	on(f)	((tmp & (f)) != 0)
#define put(n, f, d) \
	if (fmt >= BSD || on(f) != (d)) \
		bput((n) + on(f));

	/* "local" flags */
	tmp = tp->c_lflag;
	binit("lflags");
	put("-icanon", ICANON, 1);
	put("-isig", ISIG, 1);
	put("-iexten", IEXTEN, 1);
	put("-echo", ECHO, 1);
	put("-echoe", ECHOE, 0);
	put("-echok", ECHOK, 0);
	put("-echoke", ECHOKE, 0);
	put("-echonl", ECHONL, 0);
	put("-echoctl", ECHOCTL, 0);
	put("-echoprt", ECHOPRT, 0);
	put("-altwerase", ALTWERASE, 0);
	put("-noflsh", NOFLSH, 0);
	put("-tostop", TOSTOP, 0);
	put("-flusho", FLUSHO, 0);
	put("-pendin", PENDIN, 0);
	put("-nokerninfo", NOKERNINFO, 0);
	put("-extproc", EXTPROC, 0);

	/* input flags */
	tmp = tp->c_iflag;
	binit("iflags");
	put("-istrip", ISTRIP, 0);
	put("-icrnl", ICRNL, 1);
	put("-inlcr", INLCR, 0);
	put("-igncr", IGNCR, 0);
	put("-ixon", IXON, 1);
	put("-ixoff", IXOFF, 0);
	put("-ixany", IXANY, 1);
	put("-imaxbel", IMAXBEL, 1);
	put("-ignbrk", IGNBRK, 0);
	put("-brkint", BRKINT, 1);
	put("-inpck", INPCK, 0);
	put("-ignpar", IGNPAR, 0);
	put("-parmrk", PARMRK, 0);

	/* output flags */
	tmp = tp->c_oflag;
	binit("oflags");
	put("-opost", OPOST, 1);
	put("-onlcr", ONLCR, 1);
	put("-ocrnl", OCRNL, 0);
	switch(tmp&TABDLY) {
	case TAB0:
		bput("tab0");
		break;
	case TAB3:
		bput("tab3");
		break;
	}
	put("-onocr", ONOCR, 0);
	put("-onlret", ONLRET, 0);

	/* control flags (hardware state) */
	tmp = tp->c_cflag;
	binit("cflags");
	put("-cread", CREAD, 1);
	switch(tmp&CSIZE) {
	case CS5:
		bput("cs5");
		break;
	case CS6:
		bput("cs6");
		break;
	case CS7:
		bput("cs7");
		break;
	case CS8:
		bput("cs8");
		break;
	}
	bput("-parenb" + on(PARENB));
	put("-parodd", PARODD, 0);
	put("-hupcl", HUPCL, 1);
	put("-clocal", CLOCAL, 0);
	put("-cstopb", CSTOPB, 0);
	switch(tmp & (CCTS_OFLOW | CRTS_IFLOW)) {
	case CCTS_OFLOW:
		bput("ctsflow");
		break;
	case CRTS_IFLOW:
		bput("rtsflow");
		break;
	default:
		put("-crtscts", CCTS_OFLOW | CRTS_IFLOW, 0);
		break;
	}
	put("-dsrflow", CDSR_OFLOW, 0);
	put("-dtrflow", CDTR_IFLOW, 0);
	put("-mdmbuf", MDMBUF, 0);	/* XXX mdmbuf ==  dtrflow */

	/* special control characters */
	cc = tp->c_cc;
	if (fmt == POSIX) {
		binit("cchars");
		for (p = cchars1; p->name; ++p) {
			(void)snprintf(buf1, sizeof(buf1), "%s = %s;",
			    p->name, ccval(p, cc[p->sub]));
			bput(buf1);
		}
		binit(NULL);
	} else {
		binit(NULL);
		for (p = cchars1, cnt = 0; p->name; ++p) {
			if (fmt != BSD && cc[p->sub] == p->def)
				continue;
#define	WD	"%-8s"
			(void)snprintf(buf1 + cnt * 8, sizeof(buf1) - cnt * 8,
			    WD, p->name);
			(void)snprintf(buf2 + cnt * 8, sizeof(buf2) - cnt * 8,
			    WD, ccval(p, cc[p->sub]));
			if (++cnt == LINELENGTH / 8) {
				cnt = 0;
				(void)printf("%s\n", buf1);
				(void)printf("%s\n", buf2);
			}
		}
		if (cnt) {
			(void)printf("%s\n", buf1);
			(void)printf("%s\n", buf2);
		}
	}
}

static int col;
static const char *label;

static void
binit(const char *lb)
{

	if (col) {
		(void)printf("\n");
		col = 0;
	}
	label = lb;
}

static void
bput(const char *s)
{

	if (col == 0) {
		col = printf("%s: %s", label, s);
		return;
	}
	if ((col + strlen(s)) > LINELENGTH) {
		(void)printf("\n\t");
		col = printf("%s", s) + 8;
		return;
	}
	col += printf(" %s", s);
}

static const char *
ccval(struct cchar *p, int c)
{
	static char buf[5];
	char *bp;

	if (p->sub == VMIN || p->sub == VTIME) {
		(void)snprintf(buf, sizeof(buf), "%d", c);
		return (buf);
	}
	if (c == _POSIX_VDISABLE)
		return ("<undef>");
	bp = buf;
	if (c & 0200) {
		*bp++ = 'M';
		*bp++ = '-';
		c &= 0177;
	}
	if (c == 0177) {
		*bp++ = '^';
		*bp++ = '?';
	}
	else if (c < 040) {
		*bp++ = '^';
		*bp++ = c + '@';
	}
	else
		*bp++ = c;
	*bp = '\0';
	return (buf);
}