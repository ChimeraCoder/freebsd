
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
#include <stdlib.h>

#include "acl_support.h"

/*
 * acl_valid: accepts an ACL, returns 0 on valid ACL, -1 for invalid,
 * and errno set to EINVAL.
 *
 * Implemented by calling the acl_check routine in acl_support, which
 * requires ordering.  We call acl_support's _posix1e_acl_sort to make this
 * true.  POSIX.1e allows acl_valid() to reorder the ACL as it sees fit.
 *
 * This call is deprecated, as it doesn't ask whether the ACL is valid
 * for a particular target.  However, this call is standardized, unlike
 * the other two forms.
 */ 
int
acl_valid(acl_t acl)
{
	int	error;

	if (acl == NULL) {
		errno = EINVAL;
		return (-1);
	}
	if (!_acl_brand_may_be(acl, ACL_BRAND_POSIX)) {
		errno = EINVAL;
		return (-1);
	}
	_posix1e_acl_sort(acl);
	error = _posix1e_acl_check(acl);
	if (error) {
		errno = error;
		return (-1);
	} else {
		return (0);
	}
}

int
acl_valid_file_np(const char *pathp, acl_type_t type, acl_t acl)
{

	if (pathp == NULL || acl == NULL) {
		errno = EINVAL;
		return (-1);
	}
	type = _acl_type_unold(type);
	if (_posix1e_acl(acl, type))
		_posix1e_acl_sort(acl);

	return (__acl_aclcheck_file(pathp, type, &acl->ats_acl));
}

int
acl_valid_link_np(const char *pathp, acl_type_t type, acl_t acl)
{

	if (pathp == NULL || acl == NULL) {
		errno = EINVAL;
		return (-1);
	}
	type = _acl_type_unold(type);
	if (_posix1e_acl(acl, type))
		_posix1e_acl_sort(acl);

	return (__acl_aclcheck_link(pathp, type, &acl->ats_acl));
}

int
acl_valid_fd_np(int fd, acl_type_t type, acl_t acl)
{

	if (acl == NULL) {
		errno = EINVAL;
		return (-1);
	}
	type = _acl_type_unold(type);
	if (_posix1e_acl(acl, type))
		_posix1e_acl_sort(acl);

	acl->ats_cur_entry = 0;

	return (___acl_aclcheck_fd(fd, type, &acl->ats_acl));
}