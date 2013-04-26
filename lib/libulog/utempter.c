
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

#include "ulog.h"
#include "utempter.h"

static int last_fd = -1;

int
utempter_add_record(int fd, const char *host)
{

	ulog_login_pseudo(fd, host);
	last_fd = fd;
	return (0);
}

int
utempter_remove_added_record(void)
{

	if (last_fd < 0)
		return (0);
	ulog_logout_pseudo(last_fd);
	last_fd = -1;
	return (0);
}

int
utempter_remove_record(int fd)
{

	ulog_logout_pseudo(fd);
	if (last_fd == fd)
		last_fd = -1;
	return (0);
}

void
addToUtmp(const char *pty __unused, const char *host, int fd)
{

	utempter_add_record(fd, host);
}

void
removeFromUtmp(void)
{

	utempter_remove_added_record();
}

void
removeLineFromUtmp(const char *pty __unused, int fd)
{

	utempter_remove_record(fd);
}