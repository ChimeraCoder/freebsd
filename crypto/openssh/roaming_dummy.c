
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
 * This file is included in the client programs which should not
 * support roaming.
 */

#include "includes.h"

#include <sys/types.h>
#include <unistd.h>

#include "roaming.h"

int resume_in_progress = 0;

u_int64_t
get_recv_bytes(void)
{
	return 0;
}

ssize_t
roaming_write(int fd, const void *buf, size_t count, int *cont)
{
	return write(fd, buf, count);
}

ssize_t
roaming_read(int fd, void *buf, size_t count, int *cont)
{
	if (cont)
		*cont = 0;
	return read(fd, buf, count);
}

void
add_recv_bytes(u_int64_t num)
{
}

int
resume_kex(void)
{
	return 1;
}