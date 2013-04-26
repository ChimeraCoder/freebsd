
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

#define _GNU_SOURCE

#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <endian.h>
#include <byteswap.h>
#include <sys/poll.h>
#include <syslog.h>
#include <time.h>
#include <signal.h>

#include "common.h"

static int loop_on_panic;

void
stack_dump(void)
{
	if (!__builtin_frame_address(1))
		return
	syslog(LOG_ALERT, "#1 %p\n", __builtin_return_address(1));

	if (!__builtin_frame_address(2))
		return
	syslog(LOG_ALERT, "#2 %p\n", __builtin_return_address(2));

	if (!__builtin_frame_address(3))
		return
	syslog(LOG_ALERT, "#3 %p\n", __builtin_return_address(3));

	if (!__builtin_frame_address(4))
		return
	syslog(LOG_ALERT, "#4 %p\n", __builtin_return_address(4));

	if (!__builtin_frame_address(5))
		return
	syslog(LOG_ALERT, "#5 %p\n", __builtin_return_address(5));

	if (!__builtin_frame_address(6))
		return
	syslog(LOG_ALERT, "#6 %p\n", __builtin_return_address(6));

	if (!__builtin_frame_address(7))
		return
	syslog(LOG_ALERT, "#7 %p\n", __builtin_return_address(7));

	if (!__builtin_frame_address(8))
		return
	syslog(LOG_ALERT, "#8 %p\n", __builtin_return_address(8));

	if (!__builtin_frame_address(9))
		return
	syslog(LOG_ALERT, "#9 %p\n", __builtin_return_address(9));

	if (!__builtin_frame_address(10))
		return
	syslog(LOG_ALERT, "#10 %p\n", __builtin_return_address(10));

	if (!__builtin_frame_address(11))
		return
	syslog(LOG_ALERT, "#11 %p\n", __builtin_return_address(11));

	if (!__builtin_frame_address(12))
		return
	syslog(LOG_ALERT, "#12 %p\n", __builtin_return_address(12));

	if (!__builtin_frame_address(13))
		return
	syslog(LOG_ALERT, "#13 %p\n", __builtin_return_address(13));

	if (!__builtin_frame_address(14))
		return
	syslog(LOG_ALERT, "#14 %p\n", __builtin_return_address(14));

	if (!__builtin_frame_address(15))
		return
	syslog(LOG_ALERT, "#15 %p\n", __builtin_return_address(15));

	if (!__builtin_frame_address(16))
		return
	syslog(LOG_ALERT, "#16 %p\n", __builtin_return_address(16));

	if (!__builtin_frame_address(17))
		return
	syslog(LOG_ALERT, "#17 %p\n", __builtin_return_address(17));

	if (!__builtin_frame_address(18))
		return
	syslog(LOG_ALERT, "#18 %p\n", __builtin_return_address(18));
}

static void
handler(int x)
{
	static int in;
	time_t  tm;

	if (!in) {
		in++;

		syslog(LOG_ALERT, "*** exception handler: died with signal %d", x);
		stack_dump();

		fflush(NULL);

		tm = time(0);
		fprintf(stderr, "%s *** exception handler: died with signal %d pid %d\n",
				ctime(&tm), x, getpid());

		fflush(NULL);
	}

	if (loop_on_panic) {
		fprintf(stderr, "exception handler: entering tight loop ... pid %d\n",getpid());
		for (; ; )
			;
	}

	signal(x, SIG_DFL);
}

void
enable_stack_dump(int loop)
{
	loop_on_panic = loop;
	signal(SIGILL, handler);
	signal(SIGBUS, handler);
	signal(SIGSEGV, handler);
	signal(SIGABRT, handler);
}