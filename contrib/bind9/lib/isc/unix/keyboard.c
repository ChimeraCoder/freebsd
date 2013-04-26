
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

/* $Id: keyboard.c,v 1.13 2007/06/19 23:47:18 tbox Exp $ */

#include <config.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/uio.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <isc/keyboard.h>
#include <isc/util.h>

isc_result_t
isc_keyboard_open(isc_keyboard_t *keyboard) {
	int fd;
	isc_result_t ret;
	struct termios current_mode;

	REQUIRE(keyboard != NULL);

	fd = open("/dev/tty", O_RDONLY, 0);
	if (fd < 0)
		return (ISC_R_IOERROR);

	keyboard->fd = fd;

	if (tcgetattr(fd, &keyboard->saved_mode) < 0) {
		ret = ISC_R_IOERROR;
		goto errout;
	}

	current_mode = keyboard->saved_mode;

	current_mode.c_iflag &=
			~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	current_mode.c_oflag &= ~OPOST;
	current_mode.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
	current_mode.c_cflag &= ~(CSIZE|PARENB);
	current_mode.c_cflag |= CS8;

	current_mode.c_cc[VMIN] = 1;
	current_mode.c_cc[VTIME] = 0;
	if (tcsetattr(fd, TCSAFLUSH, &current_mode) < 0) {
		ret = ISC_R_IOERROR;
		goto errout;
	}

	keyboard->result = ISC_R_SUCCESS;

	return (ISC_R_SUCCESS);

 errout:
	close (fd);

	return (ret);
}

isc_result_t
isc_keyboard_close(isc_keyboard_t *keyboard, unsigned int sleeptime) {
	REQUIRE(keyboard != NULL);

	if (sleeptime > 0 && keyboard->result != ISC_R_CANCELED)
		(void)sleep(sleeptime);

	(void)tcsetattr(keyboard->fd, TCSAFLUSH, &keyboard->saved_mode);
	(void)close(keyboard->fd);

	keyboard->fd = -1;

	return (ISC_R_SUCCESS);
}

isc_result_t
isc_keyboard_getchar(isc_keyboard_t *keyboard, unsigned char *cp) {
	ssize_t cc;
	unsigned char c;
	cc_t *controlchars;

	REQUIRE(keyboard != NULL);
	REQUIRE(cp != NULL);

	cc = read(keyboard->fd, &c, 1);
	if (cc < 0) {
		keyboard->result = ISC_R_IOERROR;
		return (keyboard->result);
	}

	controlchars = keyboard->saved_mode.c_cc;
	if (c == controlchars[VINTR] || c == controlchars[VQUIT]) {
		keyboard->result = ISC_R_CANCELED;
		return (keyboard->result);
	}

	*cp = c;

	return (ISC_R_SUCCESS);
}

isc_boolean_t
isc_keyboard_canceled(isc_keyboard_t *keyboard) {
	return (ISC_TF(keyboard->result == ISC_R_CANCELED));
}