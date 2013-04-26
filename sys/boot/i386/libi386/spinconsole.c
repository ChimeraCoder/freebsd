
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

extern void get_pos(int *x, int *y);
extern void curs_move(int *_x, int *_y, int x, int y);
extern void vidc_biosputchar(int c);

static void	spinc_probe(struct console *cp);
static int	spinc_init(int arg);
static void	spinc_putchar(int c);
static int	spinc_getchar(void);
static int	spinc_ischar(void);

struct console spinconsole = {
	"spinconsole",
	"spin port",
	0,
	spinc_probe,
	spinc_init,
	spinc_putchar,
	spinc_getchar,
	spinc_ischar
};

static void
spinc_probe(struct console *cp)
{
	cp->c_flags |= (C_PRESENTIN | C_PRESENTOUT);
}

static int
spinc_init(int arg)
{
	return(0);
}

static void
spinc_putchar(int c)
{
	static int curx, cury;
	static unsigned tw_chars = 0x5C2D2F7C;    /* "\-/|" */
	static time_t lasttime;
	time_t now;

	now = time(NULL);
	if (now < (lasttime + 1))
		return;
	lasttime = now;
	get_pos(&curx, &cury);
	if (curx > 0)
		curs_move(&curx, &cury, curx - 1, cury);
	vidc_biosputchar((char)tw_chars);
	tw_chars = (tw_chars >> 8) | ((tw_chars & (unsigned long)0xFF) << 24);
}

static int
spinc_getchar(void)
{
	return(-1);
}

static int
spinc_ischar(void)
{
	return(0);
}