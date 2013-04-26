
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

/*
 * Like write but never fails (and never returns partial data).
 */

ROKEN_LIB_FUNCTION ssize_t ROKEN_LIB_CALL
ewrite (int fd, const void *buf, size_t nbytes)
{
    ssize_t ret;

    ret = net_write (fd, buf, nbytes);
    if (ret < 0)
	err (1, "write");
    return ret;
}