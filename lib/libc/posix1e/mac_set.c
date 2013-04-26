
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

#include <sys/types.h>
#include <sys/mac.h>

extern	int	__mac_set_fd(int fd, struct mac *mac_p);
extern	int	__mac_set_file(const char *path_p, struct mac *mac_p);
extern	int	__mac_set_link(const char *path_p, struct mac *mac_p);
extern	int	__mac_set_proc(struct mac *mac_p);

int
mac_set_fd(int fd, struct mac *label)
{

	return (__mac_set_fd(fd, label));
}

int
mac_set_file(const char *path, struct mac *label)
{

	return (__mac_set_file(path, label));
}

int
mac_set_link(const char *path, struct mac *label)
{

	return (__mac_set_link(path, label));
}

int
mac_set_proc(struct mac *label)
{

	return (__mac_set_proc(label));
}