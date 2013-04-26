
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

#include <sys/types.h>
#include <sys/select.h>

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

#include "inout.h"

#define	BVM_CONSOLE_PORT	0x220
#define	BVM_CONS_SIG		('b' << 8 | 'v')

static struct termios tio_orig, tio_new;

static void
ttyclose(void)
{
	tcsetattr(STDIN_FILENO, TCSANOW, &tio_orig);
}

static void
ttyopen(void)
{
	tcgetattr(STDIN_FILENO, &tio_orig);

	cfmakeraw(&tio_new);
	tcsetattr(STDIN_FILENO, TCSANOW, &tio_new);	

	atexit(ttyclose);
}

static bool
tty_char_available(void)
{
        fd_set rfds;
        struct timeval tv;

        FD_ZERO(&rfds);
        FD_SET(STDIN_FILENO, &rfds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        if (select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv) > 0) {
		return (true);
	} else {
		return (false);
	}
}

static int
ttyread(void)
{
	char rb;

	if (tty_char_available()) {
		read(STDIN_FILENO, &rb, 1);
		return (rb & 0xff);
	} else {
		return (-1);
	}
}

static void
ttywrite(unsigned char wb)
{
	(void) write(STDOUT_FILENO, &wb, 1);
}

static int
console_handler(struct vmctx *ctx, int vcpu, int in, int port, int bytes,
		uint32_t *eax, void *arg)
{
	static int opened;

	if (bytes == 2 && in) {
		*eax = BVM_CONS_SIG;
		return (0);
	}

	if (bytes != 4)
		return (-1);

	if (!opened) {
		ttyopen();
		opened = 1;
	}
	
	if (in)
		*eax = ttyread();
	else
		ttywrite(*eax);

	return (0);
}

static struct inout_port consport = {
	"bvmcons",
	BVM_CONSOLE_PORT,
	1,
	IOPORT_F_INOUT,
	console_handler
};

void
init_bvmcons(void)
{

	register_inout(&consport);
}