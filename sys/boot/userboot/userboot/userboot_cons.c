
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
#include "libuserboot.h"

int console;

static void userboot_cons_probe(struct console *cp);
static int userboot_cons_init(int);
static void userboot_cons_putchar(int);
static int userboot_cons_getchar(void);
static int userboot_cons_poll(void);

struct console userboot_console = {
	"userboot",
	"userboot",
	0,
	userboot_cons_probe,
	userboot_cons_init,
	userboot_cons_putchar,
	userboot_cons_getchar,
	userboot_cons_poll,
};

static void
userboot_cons_probe(struct console *cp)
{

	cp->c_flags |= (C_PRESENTIN | C_PRESENTOUT);
}

static int
userboot_cons_init(int arg)
{

	return (0);
}

static void
userboot_cons_putchar(int c)
{

        CALLBACK(putc, c);
}

static int
userboot_cons_getchar()
{

	return (CALLBACK(getc));
}

static int
userboot_cons_poll()
{

	return (CALLBACK(poll));
}