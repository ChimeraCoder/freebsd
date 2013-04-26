
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

#include "roken.h"

void ROKEN_LIB_FUNCTION
rk_random_init(void)
{
#if defined(HAVE_ARC4RANDOM)
    arc4random_stir();
#elif defined(HAVE_SRANDOMDEV)
    srandomdev();
#elif defined(HAVE_RANDOM)
    srandom(time(NULL));
#else
    srand (time(NULL));
#endif
}