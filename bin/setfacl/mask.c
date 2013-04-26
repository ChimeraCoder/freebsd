
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "setfacl.h"

/* set the appropriate mask the given ACL's */
int
set_acl_mask(acl_t *prev_acl, const char *filename)
{
	acl_entry_t entry;
	acl_t acl;
	acl_tag_t tag;
	int entry_id;

	entry = NULL;

	/*
	 * ... if a mask entry is specified, then the permissions of the mask
	 * entry in the resulting ACL shall be set to the permissions in the
	 * specified ACL mask entry.
	 */
	if (have_mask)
		return (0);

	acl = acl_dup(*prev_acl);
	if (acl == NULL)
		err(1, "%s: acl_dup() failed", filename);

	if (n_flag == 0) {
		/*
		 * If no mask entry is specified and the -n option is not
		 * specified, then the permissions of the resulting ACL mask
		 * entry shall be set to the union of the permissions
		 * associated with all entries which belong to the file group
		 * class in the resulting ACL
		 */
		if (acl_calc_mask(&acl)) {
			warn("%s: acl_calc_mask() failed", filename);
			acl_free(acl);
			return (-1);
		}
	} else {
		/*
		 * If no mask entry is specified and the -n option is
		 * specified, then the permissions of the resulting ACL
		 * mask entry shall remain unchanged ...
		 */

		entry_id = ACL_FIRST_ENTRY;

		while (acl_get_entry(acl, entry_id, &entry) == 1) {
			entry_id = ACL_NEXT_ENTRY;
			if (acl_get_tag_type(entry, &tag) == -1)
				err(1, "%s: acl_get_tag_type() failed",
				    filename);

			if (tag == ACL_MASK) {
				acl_free(acl);
				return (0);
			}
		}

		/*
		 * If no mask entry is specified, the -n option is specified,
		 * and no ACL mask entry exists in the ACL associated with the
		 * file, then write an error message to standard error and
		 * continue with the next file.
		 */
		warnx("%s: warning: no mask entry", filename);
		acl_free(acl);
		return (0);
	}

	acl_free(*prev_acl);
	*prev_acl = acl_dup(acl);
	acl_free(acl);

	return (0);
}