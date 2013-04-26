
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

#include <stdio.h>
#include <stdint.h>
#include <time.h>

extern int usleep(int);
extern void callout_process(int);
extern void usb_idle(void);
extern void usb_init(void);
extern void usb_uninit(void);

#define	hz 1000

void
DELAY(unsigned int delay)
{
	usleep(delay);
}

int
pause(const char *what, int timeout)
{
	if (timeout == 0)
		timeout = 1;

	usleep((1000000 / hz) * timeout);

	return (0);
}

int
main(int argc, char **argv)
{
	uint32_t time;

	usb_init();

	time = 0;

	while (1) {

		usb_idle();

		usleep(1000);

		if (++time >= (1000 / hz)) {
			time = 0;
			callout_process(1);
		}
	}

	usb_uninit();

	return (0);
}