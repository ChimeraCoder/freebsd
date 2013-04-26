
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
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "acl_support.h"

#ifndef CTASSERT
#define CTASSERT(x)		_CTASSERT(x, __LINE__)
#define _CTASSERT(x, y)		__CTASSERT(x, y)
#define __CTASSERT(x, y)	typedef char __assert_ ## y [(x) ? 1 : -1]
#endif

CTASSERT(1 << _ACL_T_ALIGNMENT_BITS > sizeof(struct acl_t_struct));

acl_t
acl_init(int count)
{
	int error;
	acl_t acl;

	if (count > ACL_MAX_ENTRIES) {
		errno = ENOMEM;
		return (NULL);
	}
	if (count < 0) {
		errno = EINVAL;
		return (NULL);
	}

	error = posix_memalign((void *)&acl, 1 << _ACL_T_ALIGNMENT_BITS,
	    sizeof(struct acl_t_struct));
	if (error)
		return (NULL);

	bzero(acl, sizeof(struct acl_t_struct));
	acl->ats_brand = ACL_BRAND_UNKNOWN;
	acl->ats_acl.acl_maxcnt = ACL_MAX_ENTRIES;

	return (acl);
}

acl_t
acl_dup(acl_t acl)
{
	acl_t	acl_new;

	acl_new = acl_init(ACL_MAX_ENTRIES);
	if (acl_new != NULL) {
		*acl_new = *acl;
		acl->ats_cur_entry = 0;
		acl_new->ats_cur_entry = 0;
	}

	return (acl_new);
}