
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
#include <bootstrap.h>

static void	nullc_probe(struct console *cp);
static int	nullc_init(int arg);
static void	nullc_putchar(int c);
static int	nullc_getchar(void);
static int	nullc_ischar(void);

struct console nullconsole = {
	"nullconsole",
	"null port",
	0,
	nullc_probe,
	nullc_init,
	nullc_putchar,
	nullc_getchar,
	nullc_ischar
};

static void
nullc_probe(struct console *cp)
{
	cp->c_flags |= (C_PRESENTIN | C_PRESENTOUT);
}

static int
nullc_init(int arg)
{
	return(0);
}

static void
nullc_putchar(int c)
{
}

static int
nullc_getchar(void)
{
	return(-1);
}

static int
nullc_ischar(void)
{
	return(0);
}