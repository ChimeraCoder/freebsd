
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
#include <sys/acl.h>
#include <sys/stat.h>

#include <err.h>
#include <stdio.h>
#include <string.h>

#include "setfacl.h"

/*
 * remove ACL entries from an ACL
 */
int
remove_acl(acl_t acl, acl_t *prev_acl, const char *filename)
{
	acl_entry_t	entry;
	acl_t		acl_new;
	acl_tag_t	tag;
	int		carried_error, entry_id, acl_brand, prev_acl_brand;

	carried_error = 0;

	acl_get_brand_np(acl, &acl_brand);
	acl_get_brand_np(*prev_acl, &prev_acl_brand);

	if (branding_mismatch(acl_brand, prev_acl_brand)) {
		warnx("%s: branding mismatch; existing ACL is %s, "
		    "entry to be removed is %s", filename,
		    brand_name(prev_acl_brand), brand_name(acl_brand));
		return (-1);
	}

	carried_error = 0;

	acl_new = acl_dup(*prev_acl);
	if (acl_new == NULL)
		err(1, "%s: acl_dup() failed", filename);

	tag = ACL_UNDEFINED_TAG;

	/* find and delete the entry */
	entry_id = ACL_FIRST_ENTRY;
	while (acl_get_entry(acl, entry_id, &entry) == 1) {
		entry_id = ACL_NEXT_ENTRY;
		if (acl_get_tag_type(entry, &tag) == -1)
			err(1, "%s: acl_get_tag_type() failed", filename);
		if (tag == ACL_MASK)
			have_mask++;
		if (acl_delete_entry(acl_new, entry) == -1) {
			carried_error++;
			warnx("%s: cannot remove non-existent ACL entry",
			    filename);
		}
	}

	acl_free(*prev_acl);
	*prev_acl = acl_new;

	if (carried_error)
		return (-1);

	return (0);
}

int
remove_by_number(uint entry_number, acl_t *prev_acl, const char *filename)
{
	acl_entry_t	entry;
	acl_t		acl_new;
	acl_tag_t	tag;
	int		carried_error, entry_id;
	uint		i;

	carried_error = 0;

	acl_new = acl_dup(*prev_acl);
	if (acl_new == NULL)
		err(1, "%s: acl_dup() failed", filename);

	tag = ACL_UNDEFINED_TAG;

	/*
	 * Find out whether we're removing the mask entry,
	 * to behave the same as the routine above.
	 *
	 * XXX: Is this loop actually needed?
	 */
	entry_id = ACL_FIRST_ENTRY;
	i = 0;
	while (acl_get_entry(acl_new, entry_id, &entry) == 1) {
		entry_id = ACL_NEXT_ENTRY;
		if (i != entry_number)
			continue;
		if (acl_get_tag_type(entry, &tag) == -1)
			err(1, "%s: acl_get_tag_type() failed", filename);
		if (tag == ACL_MASK)
			have_mask++;
	}

	if (acl_delete_entry_np(acl_new, entry_number) == -1) {
		carried_error++;
		warn("%s: acl_delete_entry_np() failed", filename);
	}

	acl_free(*prev_acl);
	*prev_acl = acl_new;

	if (carried_error)
		return (-1);

	return (0);
}

/*
 * remove default entries
 */
int
remove_default(acl_t *prev_acl, const char *filename)
{

	acl_free(*prev_acl);
	*prev_acl = acl_init(ACL_MAX_ENTRIES);
	if (*prev_acl == NULL)
		err(1, "%s: acl_init() failed", filename);

	return (0);
}

/*
 * remove extended entries
 */
void
remove_ext(acl_t *prev_acl, const char *filename)
{
	acl_t acl_new;

	acl_new = acl_strip_np(*prev_acl, !n_flag);
	if (acl_new == NULL)
		err(1, "%s: acl_strip_np() failed", filename);

	acl_free(*prev_acl);
	*prev_acl = acl_new;
}