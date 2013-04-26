
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
 * infinity.c
 */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <math.h>

/* bytes for +Infinity on a 387 */
const union __infinity_un __infinity = { 
#if BYTE_ORDER == BIG_ENDIAN
	{ 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 }
#else
	{ 0, 0, 0, 0, 0, 0, 0xf0, 0x7f }
#endif
};

/* bytes for NaN */
const union __nan_un __nan = {
#if BYTE_ORDER == BIG_ENDIAN
	{0x7f, 0xa0, 0, 0}
#else
	{ 0, 0, 0xa0, 0x7f }
#endif
};