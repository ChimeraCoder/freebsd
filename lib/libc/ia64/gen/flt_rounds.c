
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
/*
 * Written by J.T. Conklin, Apr 10, 1995
 * Public domain.
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <float.h>

static const int map[] = {
	1,	/* round to nearest */
	3,	/* round to zero */
	2,	/* round to negative infinity */
	0	/* round to positive infinity */
};

int
__flt_rounds(void)
{
	int x;

	__asm("mov %0=ar.fpsr" : "=r" (x));
        return (map[(x >> 10) & 0x03]);
}