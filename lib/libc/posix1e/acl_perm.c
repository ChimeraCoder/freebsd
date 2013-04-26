
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

#include <errno.h>
#include <string.h>

static int
_perm_is_invalid(acl_perm_t perm)
{

	/* Check if more than a single bit is set. */
	if ((perm & -perm) == perm &&
	    (perm & (ACL_POSIX1E_BITS | ACL_NFS4_PERM_BITS)) == perm)
		return (0);

	errno = EINVAL;

	return (1);
}

/*
 * acl_add_perm() (23.4.1): add the permission contained in perm to the
 * permission set permset_d
 */
int
acl_add_perm(acl_permset_t permset_d, acl_perm_t perm)
{

	if (permset_d == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (_perm_is_invalid(perm))
		return (-1);

	*permset_d |= perm;

	return (0);
}

/*
 * acl_clear_perms() (23.4.3): clear all permisions from the permission
 * set permset_d
 */
int
acl_clear_perms(acl_permset_t permset_d)
{

	if (permset_d == NULL) {
		errno = EINVAL;
		return (-1);
	}

	*permset_d = ACL_PERM_NONE;

	return (0);
}

/*
 * acl_delete_perm() (23.4.10): remove the permission in perm from the
 * permission set permset_d
 */
int
acl_delete_perm(acl_permset_t permset_d, acl_perm_t perm)
{

	if (permset_d == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (_perm_is_invalid(perm))
		return (-1);

	*permset_d &= ~perm;

	return (0);
}

int
acl_get_perm_np(acl_permset_t permset_d, acl_perm_t perm)
{

	if (permset_d == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (_perm_is_invalid(perm))
		return (-1);

	if (*permset_d & perm)
		return (1);

	return (0);
}