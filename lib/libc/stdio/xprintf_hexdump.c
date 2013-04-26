
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

#include <namespace.h>
#include <stdio.h>
#include <wchar.h>
#include <stdint.h>
#include <assert.h>
#include <sys/time.h>
#include "printf.h"

int
__printf_arginfo_hexdump(const struct printf_info *pi, size_t n, int *argt)
{

	assert(n >= 2);
	argt[0] = PA_POINTER;
	argt[1] = PA_INT;
	return (2);
}

int
__printf_render_hexdump(struct __printf_io *io, const struct printf_info *pi, const void *const *arg)
{
	unsigned char *p;
	unsigned u, l, j, a;
	char buf[100], *q;
	int ret;

	if (pi->width > 0 && pi->width < 16)
		l = pi->width;
	else
		l = 16;
	p = *((unsigned char **)arg[0]);
	u = *((unsigned *)arg[1]);

	ret = 0;
	a = 0;
	while (u > 0) {
		q = buf;
		if (pi->showsign)
			q += sprintf(q, " %04x", a);
		for (j = 0; j < l && j < u; j++)
			q += sprintf(q, " %02x", p[j]);
		if (pi->alt) {
			for (; j < l; j++)
				q += sprintf(q, "   ");
			q += sprintf(q, "  |");
			for (j = 0; j < l && j < u; j++) {
				if (p[j] < ' ' || p[j] > '~')
					*q++ = '.';
				else
					*q++ = p[j];
			}
			for (; j < l; j++)
				*q++ = ' ';
			*q++ = '|';
		}
		if (l < u)
			j = l;
		else
			j = u;
		p += j;
		u -= j;
		a += j;
		if (u > 0)
			*q++ = '\n';
		ret += __printf_puts(io, buf + 1, q - (buf + 1));
		__printf_flush(io);
	}
	return (ret);
}