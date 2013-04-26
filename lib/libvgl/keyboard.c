
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/fbio.h>
#include <sys/kbio.h>
#include "vgl.h"

static struct termios VGLKeyboardTty;
static int VGLKeyboardMode = -1;

int
VGLKeyboardInit(int mode)
{
  static struct termios term;

  ioctl(0, KDGKBMODE, &VGLKeyboardMode);
  tcgetattr(0, &VGLKeyboardTty);

  term = VGLKeyboardTty;
  cfmakeraw(&term);
  term.c_iflag = IGNPAR | IGNBRK;
  term.c_oflag = OPOST | ONLCR;
  term.c_cflag = CREAD | CS8;
  term.c_lflag &= ~(ICANON | ECHO | ISIG);
  term.c_cc[VTIME] = 0;
  term.c_cc[VMIN] = 0;
  cfsetispeed(&term, 9600);
  cfsetospeed(&term, 9600);
  tcsetattr(0, TCSANOW | TCSAFLUSH, &term);

  switch (mode) {
  case VGL_RAWKEYS:
    ioctl(0, KDSKBMODE, K_RAW);
    break;
  case VGL_CODEKEYS:
    ioctl(0, KDSKBMODE, K_CODE);
    break;
  case VGL_XLATEKEYS:
    ioctl(0, KDSKBMODE, K_XLATE);
    break;
  }
  return 0;
}

void
VGLKeyboardEnd()
{
  if (VGLKeyboardMode != -1) {
    ioctl(0, KDSKBMODE, VGLKeyboardMode);
    tcsetattr(0, TCSANOW | TCSAFLUSH, &VGLKeyboardTty);
  }
}

int
VGLKeyboardGetCh()
{
  unsigned char ch = 0;

  read (0, &ch, 1);
  return (int)ch;
}