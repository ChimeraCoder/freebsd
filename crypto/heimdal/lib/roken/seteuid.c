
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "roken.h"

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
seteuid(uid_t euid)
{
#ifdef HAVE_SETREUID
    return setreuid(-1, euid);
#endif

#ifdef HAVE_SETRESUID
    return setresuid(-1, euid, -1);
#endif

    return -1;
}