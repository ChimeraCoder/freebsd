
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
__printf_arginfo_errno(const struct printf_info *pi __unused, size_t n, int *argt)
{

	assert(n >= 1);
	argt[0] = PA_INT;
	return (1);
}

int
__printf_render_errno(struct __printf_io *io, const struct printf_info *pi __unused, const void *const *arg)
{
	int ret, error;
	char buf[64];
	const char *p;

	ret = 0;
	error = *((const int *)arg[0]);
	if (error >= 0 && error < sys_nerr) {
		p = strerror(error);
		return (__printf_out(io, pi, p, strlen(p)));
	}
	sprintf(buf, "errno=%d/0x%x", error, error);
	ret += __printf_out(io, pi, buf, strlen(buf));
	__printf_flush(io);
	return(ret);
}