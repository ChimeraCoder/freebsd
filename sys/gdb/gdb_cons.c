
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

/* 
 * Support for redirecting console msgs to gdb.  We register
 * a pseudo console to hook cnputc and send stuff to the gdb
 * port.  The only trickiness here is buffering output so this
 * isn't dog slow.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/cons.h>
#include <sys/kdb.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/reboot.h>
#include <sys/sysctl.h>

#include <machine/gdb_machdep.h>
#include <machine/kdb.h>

#include <gdb/gdb.h>
#include <gdb/gdb_int.h>

struct gdbcons {
	int	npending;
	/* /2 for hex conversion, -6 for protocol glue */
	char	buf[GDB_BUFSZ/2 - 6];
	struct callout flush;
};
static struct gdbcons state = { -1 };

static	int gdbcons_enable = 0;
SYSCTL_INT(_debug, OID_AUTO, gdbcons, CTLFLAG_RW, &gdbcons_enable,
	    0, "copy console messages to gdb");
TUNABLE_INT("debug.gdbcons", &gdbcons_enable);

static void
gdb_cnprobe(struct consdev *cp)
{
	sprintf(cp->cn_name, "gdb");
	cp->cn_pri = CN_LOW;		/* XXX no way to say "write only" */
}

static void
gdb_cninit(struct consdev *cp)
{
	struct gdbcons *c = &state;

	/* setup tx buffer and callout */
	if (c->npending == -1) {
		c->npending = 0;
		callout_init(&c->flush, CALLOUT_MPSAFE);
		cp->cn_arg = c;
	}
}

static void
gdb_cnterm(struct consdev *cp)
{
}

static void
gdb_cngrab(struct consdev *cp)
{
}

static void
gdb_cnungrab(struct consdev *cp)
{
}

static int
gdb_cngetc(struct consdev *cp)
{
	return -1;
}

static void
gdb_tx_puthex(int c)
{
	const char *hex = "0123456789abcdef";

	gdb_tx_char(hex[(c>>4)&0xf]);
	gdb_tx_char(hex[(c>>0)&0xf]);
}

static void
gdb_cnflush(void *arg)
{
	struct gdbcons *gc = arg;
	int i;

	gdb_tx_begin('O');
	for (i = 0; i < gc->npending; i++)
		gdb_tx_puthex(gc->buf[i]);
	gdb_tx_end();
	gc->npending = 0;
}

/*
 * This glop is to figure out when it's safe to use callouts
 * to defer buffer flushing.  There's probably a better way
 * and/or an earlier point in the boot process when it's ok.
 */
static int calloutok = 0;
static void
oktousecallout(void *data __unused)
{
	calloutok = 1;
}
SYSINIT(gdbhack, SI_SUB_RUN_SCHEDULER, SI_ORDER_MIDDLE, oktousecallout, NULL);

static void
gdb_cnputc(struct consdev *cp, int c)
{
	struct gdbcons *gc;

	if (gdbcons_enable && gdb_cur != NULL && gdb_listening) {
		gc = cp->cn_arg;
		if (gc->npending != 0) {
			/*
			 * Cancel any pending callout and flush the
			 * buffer if there's no space for this byte.
			 */
			if (calloutok)
				callout_stop(&gc->flush);
			if (gc->npending == sizeof(gc->buf))
				gdb_cnflush(gc);
		}
		gc->buf[gc->npending++] = c;
		/*
		 * Flush on end of line; this is especially helpful
		 * during boot when we don't have callouts to flush
		 * the buffer.  Otherwise we defer flushing; a 1/4 
		 * second is a guess.
		 */
		if (c == '\n')
			gdb_cnflush(gc);
		else if (calloutok)
			callout_reset(&gc->flush, hz/4, gdb_cnflush, gc);
	}
}

CONSOLE_DRIVER(gdb);

/*
 * Our console device only gets attached if the system is booted
 * with RB_MULTIPLE set so gdb_init also calls us to attach the
 * console so we're setup regardless.
 */
void
gdb_consinit(void)
{
	gdb_cnprobe(&gdb_consdev);
	gdb_cninit(&gdb_consdev);
	cnadd(&gdb_consdev);
}