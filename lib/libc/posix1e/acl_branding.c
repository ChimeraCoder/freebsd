
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

#include <assert.h>
#include <errno.h>
#include <sys/acl.h>

#include "acl_support.h"

/*
 * An ugly detail of the implementation - fortunately not visible
 * to the API users - is the "branding": libc needs to keep track
 * of what "brand" ACL is: NFSv4, POSIX.1e or unknown.  It happens
 * automatically - for example, during acl_get_file(3) ACL gets
 * branded according to the "type" argument; during acl_set_permset
 * ACL, if its brand is unknown it gets branded as NFSv4 if any of the
 * NFSv4 permissions that are not valid for POSIX.1e ACL are set etc.
 * Branding information is used for printing out the ACL (acl_to_text(3)),
 * veryfying acl_set_whatever arguments (checking against setting
 * bits that are valid only for NFSv4 in ACL branded as POSIX.1e) etc.
 */

static acl_t
entry2acl(acl_entry_t entry)
{
	acl_t aclp;

	aclp = (acl_t)(((long)entry >> _ACL_T_ALIGNMENT_BITS) << _ACL_T_ALIGNMENT_BITS);

	return (aclp);
}

/*
 * Return brand of an ACL.
 */
int
_acl_brand(const acl_t acl)
{

	return (acl->ats_brand);
}

int
_entry_brand(const acl_entry_t entry)
{

	return (_acl_brand(entry2acl(entry)));
}

/*
 * Return 1, iff branding ACL as "brand" is ok.
 */
int
_acl_brand_may_be(const acl_t acl, int brand)
{

	if (_acl_brand(acl) == ACL_BRAND_UNKNOWN)
		return (1);

	if (_acl_brand(acl) == brand)
		return (1);

	return (0);
}

int
_entry_brand_may_be(const acl_entry_t entry, int brand)
{

	return (_acl_brand_may_be(entry2acl(entry), brand));
}

/*
 * Brand ACL as "brand".
 */
void
_acl_brand_as(acl_t acl, int brand)
{

	assert(_acl_brand_may_be(acl, brand));

	acl->ats_brand = brand;
}

void
_entry_brand_as(const acl_entry_t entry, int brand)
{

	_acl_brand_as(entry2acl(entry), brand);
}

int
_acl_type_not_valid_for_acl(const acl_t acl, acl_type_t type)
{

	switch (_acl_brand(acl)) {
	case ACL_BRAND_NFS4:
		if (type == ACL_TYPE_NFS4)
			return (0);
		break;

	case ACL_BRAND_POSIX:
		if (type == ACL_TYPE_ACCESS || type == ACL_TYPE_DEFAULT)
			return (0);
		break;

	case ACL_BRAND_UNKNOWN:
		return (0);
	}

	return (-1);
}

void
_acl_brand_from_type(acl_t acl, acl_type_t type)
{

	switch (type) {
	case ACL_TYPE_NFS4:
		_acl_brand_as(acl, ACL_BRAND_NFS4);
		break;
	case ACL_TYPE_ACCESS:
	case ACL_TYPE_DEFAULT:
		_acl_brand_as(acl, ACL_BRAND_POSIX);
		break;
	default:
		/* XXX: What to do here? */
		break;
	}
}

int
acl_get_brand_np(acl_t acl, int *brand_p)
{

	if (acl == NULL || brand_p == NULL) {
		errno = EINVAL;
		return (-1);
	}
	*brand_p = _acl_brand(acl);

	return (0);
}