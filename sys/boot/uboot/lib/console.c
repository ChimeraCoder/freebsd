
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

#include <stand.h>
#include "bootstrap.h"
#include "glue.h"

int console;

static void uboot_cons_probe(struct console *cp);
static int uboot_cons_init(int);
static void uboot_cons_putchar(int);
static int uboot_cons_getchar(void);
static int uboot_cons_poll(void);

struct console uboot_console = {
	"uboot",
	"U-Boot console",
	0,
	uboot_cons_probe,
	uboot_cons_init,
	uboot_cons_putchar,
	uboot_cons_getchar,
	uboot_cons_poll,
};

static void
uboot_cons_probe(struct console *cp)
{

	cp->c_flags |= (C_PRESENTIN | C_PRESENTOUT);
}

static int
uboot_cons_init(int arg)
{

	return (0);
}

static void
uboot_cons_putchar(int c)
{

	if (c == '\n')
		ub_putc('\r');

	ub_putc(c);
}

static int
uboot_cons_getchar()
{

	return (ub_getc());
}

static int
uboot_cons_poll()
{

	return (ub_tstc());
}