
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

#include <config.h>
#include "roken.h"

#ifndef HAVE_BSWAP32

ROKEN_LIB_FUNCTION unsigned int ROKEN_LIB_CALL
bswap32 (unsigned int val)
{
    return (val & 0xff) << 24 |
	(val & 0xff00) << 8 |
	(val & 0xff0000) >> 8 |
	(val & 0xff000000) >> 24;
}
#endif

#ifndef HAVE_BSWAP16

ROKEN_LIB_FUNCTION unsigned short ROKEN_LIB_CALL
bswap16 (unsigned short val)
{
    return (val & 0xff) << 8 |
	(val & 0xff00) >> 8;
}
#endif