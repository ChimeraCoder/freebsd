
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

#include "includes.h"

#include <sys/types.h>
#include <stdio.h>
#include <termios.h>
#include <pwd.h>

#include "sshpty.h"

static struct termios _saved_tio;
static int _in_raw_mode = 0;

struct termios *
get_saved_tio(void)
{
	return _in_raw_mode ? &_saved_tio : NULL;
}

void
leave_raw_mode(int quiet)
{
	if (!_in_raw_mode)
		return;
	if (tcsetattr(fileno(stdin), TCSADRAIN, &_saved_tio) == -1) {
		if (!quiet)
			perror("tcsetattr");
	} else
		_in_raw_mode = 0;
}

void
enter_raw_mode(int quiet)
{
	struct termios tio;

	if (tcgetattr(fileno(stdin), &tio) == -1) {
		if (!quiet)
			perror("tcgetattr");
		return;
	}
	_saved_tio = tio;
	tio.c_iflag |= IGNPAR;
	tio.c_iflag &= ~(ISTRIP | INLCR | IGNCR | ICRNL | IXON | IXANY | IXOFF);
#ifdef IUCLC
	tio.c_iflag &= ~IUCLC;
#endif
	tio.c_lflag &= ~(ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHONL);
#ifdef IEXTEN
	tio.c_lflag &= ~IEXTEN;
#endif
	tio.c_oflag &= ~OPOST;
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;
	if (tcsetattr(fileno(stdin), TCSADRAIN, &tio) == -1) {
		if (!quiet)
			perror("tcsetattr");
	} else
		_in_raw_mode = 1;
}