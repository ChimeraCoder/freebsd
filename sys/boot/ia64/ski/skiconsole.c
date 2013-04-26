
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
#include "libski.h"

static void
ski_cons_probe(struct console *cp)
{
	cp->c_flags |= C_PRESENTIN | C_PRESENTOUT;
}

static int
ski_cons_init(int arg)
{
	ssc(0, 0, 0, 0, SSC_CONSOLE_INIT);
	return 0;
}

void
ski_cons_putchar(int c)
{
	ssc(c, 0, 0, 0, SSC_PUTCHAR);
}

static int pollchar = -1;

int
ski_cons_getchar()
{
	int c;

	if (pollchar > 0) {
		c = pollchar;
		pollchar = -1;
		return c;
	}

	do {
		c = ssc(0, 0, 0, 0, SSC_GETCHAR);
	} while (c == 0);

	return c;
}

int
ski_cons_poll()
{
	int c;
	if (pollchar > 0)
		return 1;
	c = ssc(0, 0, 0, 0, SSC_GETCHAR);
	if (!c)
		return 0;
	pollchar = c;
	return 1;
}

struct console ski_console = {
	"ski",
	"ia64 SKI console",
	0,
	ski_cons_probe,
	ski_cons_init,
	ski_cons_putchar,
	ski_cons_getchar,
	ski_cons_poll
};