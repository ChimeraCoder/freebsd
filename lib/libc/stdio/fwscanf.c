
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

#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>
#include <xlocale.h>

int
fwscanf(FILE * __restrict fp, const wchar_t * __restrict fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = vfwscanf(fp, fmt, ap);
	va_end(ap);

	return (r);
}
int
fwscanf_l(FILE * __restrict fp, locale_t locale, const wchar_t * __restrict fmt, ...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = vfwscanf_l(fp, locale, fmt, ap);
	va_end(ap);

	return (r);
}