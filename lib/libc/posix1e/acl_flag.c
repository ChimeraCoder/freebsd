
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

#include <stdio.h>
#include <errno.h>
#include <sys/acl.h>

#include "acl_support.h"

static int
_flag_is_invalid(acl_flag_t flag)
{

	if ((flag & ACL_FLAGS_BITS) == flag)
		return (0);

	errno = EINVAL;

	return (1);
}

int
acl_add_flag_np(acl_flagset_t flagset_d, acl_flag_t flag)
{

	if (flagset_d == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (_flag_is_invalid(flag))
		return (-1);

	*flagset_d |= flag;

	return (0);
}

int
acl_clear_flags_np(acl_flagset_t flagset_d)
{

	if (flagset_d == NULL) {
		errno = EINVAL;
		return (-1);
	}

	*flagset_d |= 0;

	return (0);
}

int
acl_delete_flag_np(acl_flagset_t flagset_d, acl_flag_t flag)
{

	if (flagset_d == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (_flag_is_invalid(flag))
		return (-1);

	*flagset_d &= ~flag;

	return (0);
}

int
acl_get_flag_np(acl_flagset_t flagset_d, acl_flag_t flag)
{

	if (flagset_d == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (_flag_is_invalid(flag))
		return (-1);

	if (*flagset_d & flag)
		return (1);

	return (0);
}

int
acl_get_flagset_np(acl_entry_t entry_d, acl_flagset_t *flagset_p)
{

	if (entry_d == NULL || flagset_p == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (!_entry_brand_may_be(entry_d, ACL_BRAND_NFS4)) {
		errno = EINVAL;
		return (-1);
	}

	*flagset_p = &entry_d->ae_flags;

	return (0);
}

int
acl_set_flagset_np(acl_entry_t entry_d, acl_flagset_t flagset_d)
{

	if (entry_d == NULL) {
		errno = EINVAL;
		return (-1);
	}

	if (!_entry_brand_may_be(entry_d, ACL_BRAND_NFS4)) {
		errno = EINVAL;
		return (-1);
	}

	_entry_brand_as(entry_d, ACL_BRAND_NFS4);

	if (_flag_is_invalid(*flagset_d))
		return (-1);

	entry_d->ae_flags = *flagset_d;

	return (0);
}