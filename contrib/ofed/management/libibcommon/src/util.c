
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
#include <netinet/in.h>

#include <common.h>

void
ibwarn(const char * const fn, char *msg, ...)
{
	char buf[512];
	va_list va;
	int n;

	va_start(va, msg);
	n = vsnprintf(buf, sizeof(buf), msg, va);
	va_end(va);

	printf("ibwarn: [%d] %s: %s\n", getpid(), fn, buf);
}

void
ibpanic(const char * const fn, char *msg, ...)
{
	char buf[512];
	va_list va;
	int n;

	va_start(va, msg);
	n = vsnprintf(buf, sizeof(buf), msg, va);
	va_end(va);

	printf("ibpanic: [%d] %s: %s: (%m)\n", getpid(), fn, buf);
	syslog(LOG_ALERT, "ibpanic: [%d] %s: %s: (%m)\n", getpid(), fn, buf);

	exit(-1);
}

void
logmsg(const char * const fn, char *msg, ...)
{
	char buf[512];
	va_list va;
	int n;

	va_start(va, msg);
	n = vsnprintf(buf, sizeof(buf), msg, va);
	va_end(va);

	syslog(LOG_ALERT, "[%d] %s: %s: (%m)\n", getpid(), fn, buf);
}

void
xdump(FILE *file, char *msg, void *p, int size)
{
#define HEX(x)  ((x) < 10 ? '0' + (x) : 'a' + ((x) -10))
        uint8_t *cp = p;
        int i;

	if (msg)
		fputs(msg, file);

        for (i = 0; i < size;) {
                fputc(HEX(*cp >> 4), file);
                fputc(HEX(*cp & 0xf), file);
                if (++i >= size)
                        break;
                fputc(HEX(cp[1] >> 4), file);
                fputc(HEX(cp[1] & 0xf), file);
                if ((++i) % 16)
                        fputc(' ', file);
                else
                        fputc('\n', file);
                cp += 2;
        }
        if (i % 16) {
                fputc('\n', file);
        }
}