
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
#include "namespace.h"
#include <sys/acl.h>
#include "un-namespace.h"
#include <sys/errno.h>

#include "acl_support.h"

int
acl_delete_def_file(const char *path_p)
{

	return (__acl_delete_file(path_p, ACL_TYPE_DEFAULT));
}

int
acl_delete_def_link_np(const char *path_p)
{

	return (__acl_delete_link(path_p, ACL_TYPE_DEFAULT));
}

int
acl_delete_file_np(const char *path_p, acl_type_t type)
{

	type = _acl_type_unold(type);
	return (__acl_delete_file(path_p, type));
}

int
acl_delete_link_np(const char *path_p, acl_type_t type)
{

	type = _acl_type_unold(type);
	return (__acl_delete_link(path_p, type));
}

int
acl_delete_fd_np(int filedes, acl_type_t type)
{

	type = _acl_type_unold(type);
	return (___acl_delete_fd(filedes, type));
}