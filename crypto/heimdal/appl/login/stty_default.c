
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

#include "login_locl.h"

RCSID("$Id$");

#include <termios.h>

/* HP-UX 9.0 termios doesn't define these */
#ifndef FLUSHO
#define	FLUSHO	0
#endif

#ifndef XTABS
#define	XTABS	0
#endif

#ifndef OXTABS
#define OXTABS	XTABS
#endif

/* Ultrix... */
#ifndef ECHOPRT
#define ECHOPRT	0
#endif

#ifndef ECHOCTL
#define ECHOCTL	0
#endif

#ifndef ECHOKE
#define ECHOKE	0
#endif

#ifndef IMAXBEL
#define IMAXBEL	0
#endif

#define Ctl(x) ((x) ^ 0100)

void
stty_default(void)
{
    struct	termios termios;

    /*
     * Finalize the terminal settings. Some systems default to 8 bits,
     * others to 7, so we should leave that alone.
     */
    tcgetattr(0, &termios);

    termios.c_iflag |= (BRKINT|IGNPAR|ICRNL|IXON|IMAXBEL);
    termios.c_iflag &= ~IXANY;

    termios.c_lflag |= (ISIG|IEXTEN|ICANON|ECHO|ECHOE|ECHOK|ECHOCTL|ECHOKE);
    termios.c_lflag &= ~(ECHOPRT|TOSTOP|FLUSHO);

    termios.c_oflag |= (OPOST|ONLCR);
    termios.c_oflag &= ~OXTABS;

    termios.c_cc[VINTR] = Ctl('C');
    termios.c_cc[VERASE] = Ctl('H');
    termios.c_cc[VKILL] = Ctl('U');
    termios.c_cc[VEOF] = Ctl('D');

    termios.c_cc[VSUSP] = Ctl('Z');

    tcsetattr(0, TCSANOW, &termios);
}