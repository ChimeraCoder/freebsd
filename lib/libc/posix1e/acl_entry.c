
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

/*
 * acl_create_entry() (23.4.7): create a new ACL entry in the ACL pointed
 * to by acl_p.
 */
int
acl_create_entry(acl_t *acl_p, acl_entry_t *entry_p)
{
	struct acl *acl_int;

	if (acl_p == NULL) {
		errno = EINVAL;
		return (-1);
	}

	acl_int = &(*acl_p)->ats_acl;

	/*
	 * +1, because we are checking if there is space left for one more
	 * entry.
	 */
	if (acl_int->acl_cnt + 1 >= ACL_MAX_ENTRIES) {
		errno = EINVAL;
		return (-1);
	}

	*entry_p = &acl_int->acl_entry[acl_int->acl_cnt++];

	(**entry_p).ae_tag  = ACL_UNDEFINED_TAG;
	(**entry_p).ae_id   = ACL_UNDEFINED_ID;
	(**entry_p).ae_perm = ACL_PERM_NONE;
	(**entry_p).ae_entry_type = 0;
	(**entry_p).ae_flags = 0;

	(*acl_p)->ats_cur_entry = 0;

	return (0);
}

int
acl_create_entry_np(acl_t *acl_p, acl_entry_t *entry_p, int offset)
{
	int i;
	struct acl *acl_int;

	if (acl_p == NULL) {
		errno = EINVAL;
		return (-1);
	}

	acl_int = &(*acl_p)->ats_acl;

	if (acl_int->acl_cnt + 1 >= ACL_MAX_ENTRIES) {
		errno = EINVAL;
		return (-1);
	}

	if (offset < 0 || offset >= acl_int->acl_cnt) {
		errno = EINVAL;
		return (-1);
	}

	/* Make room for the new entry. */
	for (i = acl_int->acl_cnt; i > offset; i--)
		acl_int->acl_entry[i] = acl_int->acl_entry[i - 1];

	acl_int->acl_cnt++;

	*entry_p = &acl_int->acl_entry[offset];

	(**entry_p).ae_tag  = ACL_UNDEFINED_TAG;
	(**entry_p).ae_id   = ACL_UNDEFINED_ID;
	(**entry_p).ae_perm = ACL_PERM_NONE;
	(**entry_p).ae_entry_type = 0;
	(**entry_p).ae_flags= 0;

	(*acl_p)->ats_cur_entry = 0;

	return (0);
}

/*
 * acl_get_entry() (23.4.14): returns an ACL entry from an ACL
 * indicated by entry_id.
 */
int
acl_get_entry(acl_t acl, int entry_id, acl_entry_t *entry_p)
{
	struct acl *acl_int;

	if (acl == NULL) {
		errno = EINVAL;
		return (-1);
	}
	acl_int = &acl->ats_acl;

	switch(entry_id) {
	case ACL_FIRST_ENTRY:
		acl->ats_cur_entry = 0;
		/* PASSTHROUGH */
	case ACL_NEXT_ENTRY:
		if (acl->ats_cur_entry >= acl->ats_acl.acl_cnt)
			return 0;
		*entry_p = &acl_int->acl_entry[acl->ats_cur_entry++];
		return (1);
	}

	errno = EINVAL;
	return (-1);
}