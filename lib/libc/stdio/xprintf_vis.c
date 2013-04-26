
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
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <vis.h>
#include <assert.h>
#include <sys/time.h>
#include "printf.h"

int
__printf_arginfo_vis(const struct printf_info *pi, size_t n, int *argt)
{

	assert(n >= 1);
	argt[0] = PA_POINTER;
	return (1);
}

int
__printf_render_vis(struct __printf_io *io, const struct printf_info *pi, const void *const *arg)
{
	char *p, *buf;
	unsigned l;
	int ret;

	ret = 0;
	p = *((char **)arg[0]);
	if (p == NULL)
		return (__printf_out(io, pi, "(null)", 6));
	if (pi->prec >= 0)
		l = pi->prec;
	else
		l = strlen(p);
	buf = malloc(l * 4 + 1);
	if (buf == NULL)
		return (-1);
	if (pi->showsign)
		ret = strvisx(buf, p, l, VIS_WHITE | VIS_HTTPSTYLE);
	else if (pi->pad == '0')
		ret = strvisx(buf, p, l, VIS_WHITE | VIS_OCTAL);
	else if (pi->alt)
		ret = strvisx(buf, p, l, VIS_WHITE);
	else
		ret = strvisx(buf, p, l, VIS_WHITE | VIS_CSTYLE | VIS_OCTAL);
	ret += __printf_out(io, pi, buf, ret);
	__printf_flush(io);
	free(buf);
	return(ret);
}