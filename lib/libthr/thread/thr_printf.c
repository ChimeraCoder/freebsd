
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

#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "thr_private.h"

static void	pchar(int fd, char c);
static void	pstr(int fd, const char *s);

/*
 * Write formatted output to stdout, in a thread-safe manner.
 *
 * Recognises the following conversions:
 *	%c	-> char
 *	%d	-> signed int (base 10)
 *	%s	-> string
 *	%u	-> unsigned int (base 10)
 *	%x	-> unsigned int (base 16)
 *	%p	-> unsigned int (base 16)
 */
void
_thread_printf(int fd, const char *fmt, ...)
{
	static const char digits[16] = "0123456789abcdef";
	va_list	 ap;
	char buf[20];
	char *s;
	unsigned long r, u;
	int c;
	long d;
	int islong;

	va_start(ap, fmt);
	while ((c = *fmt++)) {
		islong = 0;
		if (c == '%') {
next:			c = *fmt++;
			if (c == '\0')
				goto out;
			switch (c) {
			case 'c':
				pchar(fd, va_arg(ap, int));
				continue;
			case 's':
				pstr(fd, va_arg(ap, char *));
				continue;
			case 'l':
				islong = 1;
				goto next;
			case 'p':
				islong = 1;
			case 'd':
			case 'u':
			case 'x':
				r = ((c == 'u') || (c == 'd')) ? 10 : 16;
				if (c == 'd') {
					if (islong)
						d = va_arg(ap, unsigned long);
					else
						d = va_arg(ap, unsigned);
					if (d < 0) {
						pchar(fd, '-');
						u = (unsigned long)(d * -1);
					} else
						u = (unsigned long)d;
				} else {
					if (islong)
						u = va_arg(ap, unsigned long);
					else
						u = va_arg(ap, unsigned);
				}
				s = buf;
				do {
					*s++ = digits[u % r];
				} while (u /= r);
				while (--s >= buf)
					pchar(fd, *s);
				continue;
			}
		}
		pchar(fd, c);
	}
out:	
	va_end(ap);
}

/*
 * Write a single character to stdout, in a thread-safe manner.
 */
static void
pchar(int fd, char c)
{

	__sys_write(fd, &c, 1);
}

/*
 * Write a string to stdout, in a thread-safe manner.
 */
static void
pstr(int fd, const char *s)
{

	__sys_write(fd, s, strlen(s));
}