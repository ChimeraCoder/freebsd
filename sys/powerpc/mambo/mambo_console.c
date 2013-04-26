
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
#include <sys/kdb.h>
#include <sys/kernel.h>
#include <sys/priv.h>
#include <sys/systm.h>
#include <sys/types.h>
#include <sys/conf.h>
#include <sys/cons.h>
#include <sys/consio.h>
#include <sys/tty.h>

#include <dev/ofw/openfirm.h>

#include <ddb/ddb.h>

#include "mambocall.h"

#define MAMBOBURSTLEN	128	/* max number of bytes to write in one chunk */

#define MAMBO_CONSOLE_WRITE	0
#define MAMBO_CONSOLE_READ	60

static tsw_outwakeup_t mambotty_outwakeup;

static struct ttydevsw mambo_ttydevsw = {
	.tsw_flags	= TF_NOPREFIX,
	.tsw_outwakeup	= mambotty_outwakeup,
};

static int			polltime;
static struct callout		mambo_callout;
static struct tty 		*tp = NULL;

#if defined(KDB)
static int			alt_break_state;
#endif

static void	mambo_timeout(void *);

static cn_probe_t	mambo_cnprobe;
static cn_init_t	mambo_cninit;
static cn_term_t	mambo_cnterm;
static cn_getc_t	mambo_cngetc;
static cn_putc_t	mambo_cnputc;
static cn_grab_t	mambo_cngrab;
static cn_ungrab_t	mambo_cnungrab;

CONSOLE_DRIVER(mambo);

static void
cn_drvinit(void *unused)
{

	if (mambo_consdev.cn_pri != CN_DEAD &&
	    mambo_consdev.cn_name[0] != '\0') {
		if (OF_finddevice("/mambo") == -1)
			return;

		tp = tty_alloc(&mambo_ttydevsw, NULL);
		tty_init_console(tp, 0);
		tty_makedev(tp, NULL, "%s", "mambocons");

		polltime = 1;

		callout_init(&mambo_callout, CALLOUT_MPSAFE);
		callout_reset(&mambo_callout, polltime, mambo_timeout, NULL);
	}
}

SYSINIT(cndev, SI_SUB_CONFIGURE, SI_ORDER_MIDDLE, cn_drvinit, NULL);

static void
mambotty_outwakeup(struct tty *tp)
{
	int len;
	u_char buf[MAMBOBURSTLEN];

	for (;;) {
		len = ttydisc_getc(tp, buf, sizeof buf);
		if (len == 0)
			break;
		mambocall(MAMBO_CONSOLE_WRITE, buf, (register_t)len, 1UL);
	}
}

static void
mambo_timeout(void *v)
{
	int 	c;

	tty_lock(tp);
	while ((c = mambo_cngetc(NULL)) != -1)
		ttydisc_rint(tp, c, 0);
	ttydisc_rint_done(tp);
	tty_unlock(tp);

	callout_reset(&mambo_callout, polltime, mambo_timeout, NULL);
}

static void
mambo_cnprobe(struct consdev *cp)
{
	if (OF_finddevice("/mambo") == -1) {
		cp->cn_pri = CN_DEAD;
		return;
	}

	cp->cn_pri = CN_NORMAL;
}

static void
mambo_cninit(struct consdev *cp)
{

	/* XXX: This is the alias, but that should be good enough */
	strcpy(cp->cn_name, "mambocons");
}

static void
mambo_cnterm(struct consdev *cp)
{
}

static void
mambo_cngrab(struct consdev *cp)
{
}

static void
mambo_cnungrab(struct consdev *cp)
{
}

static int
mambo_cngetc(struct consdev *cp)
{
	int ch;

	ch = mambocall(MAMBO_CONSOLE_READ);

	if (ch > 0 && ch < 0xff) {
#if defined(KDB)
		kdb_alt_break(ch, &alt_break_state);
#endif
		return (ch);
	}

	return (-1);
}

static void
mambo_cnputc(struct consdev *cp, int c)
{
	char cbuf;

	cbuf = c;
	mambocall(MAMBO_CONSOLE_WRITE, &cbuf, 1UL, 1UL);
}