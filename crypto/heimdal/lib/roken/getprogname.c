
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

#ifndef HAVE___PROGNAME
const char *__progname;
#endif

#ifndef HAVE_GETPROGNAME
ROKEN_LIB_FUNCTION const char * ROKEN_LIB_CALL
getprogname(void)
{
    return __progname;
}
#endif /* HAVE_GETPROGNAME */