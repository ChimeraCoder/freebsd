
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
#include <ctype.h>
#include <wchar.h>
#include <vis.h>
#include <assert.h>
#include <sys/time.h>
#include "printf.h"

int
__printf_arginfo_quote(const struct printf_info *pi __unused, size_t n, int *argt)
{

	assert(n >= 1);
	argt[0] = PA_POINTER;
	return (1);
}

int
__printf_render_quote(struct __printf_io *io, const struct printf_info *pi __unused, const void *const *arg)
{
	const char *str, *p, *t, *o;
	char r[5];
	int i, ret;

	str = *((const char *const *)arg[0]);
	if (str == NULL)
		return (__printf_out(io, pi, "\"(null)\"", 8));
	if (*str == '\0')
		return (__printf_out(io, pi, "\"\"", 2));

	for (i = 0, p = str; *p; p++)
		if (isspace(*p) || *p == '\\' || *p == '"')
			i++;
	if (!i) 
		return (__printf_out(io, pi, str, strlen(str)));
	
	ret = __printf_out(io, pi, "\"", 1);
	for (t = p = str; *p; p++) {
		o = NULL;
		if (*p == '\\')
			o = "\\\\";
		else if (*p == '\n')
			o = "\\n";
		else if (*p == '\r')
			o = "\\r";
		else if (*p == '\t')
			o = "\\t";
		else if (*p == ' ')
			o = " ";
		else if (*p == '"')
			o = "\\\"";
		else if (isspace(*p)) {
			sprintf(r, "\\%03o", *p);
			o = r;
		} else
			continue;
		if (p != t)
			ret += __printf_out(io, pi, t, p - t);
		ret += __printf_out(io, pi, o, strlen(o));
		t = p + 1;
	}
	if (p != t)
		ret += __printf_out(io, pi, t, p - t);
	ret += __printf_out(io, pi, "\"", 1);
	__printf_flush(io);
	return(ret);
}