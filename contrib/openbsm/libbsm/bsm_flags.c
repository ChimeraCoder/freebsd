
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

#include <config/config.h>

#include <bsm/libbsm.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifndef HAVE_STRLCPY
#include <compat/strlcpy.h>
#endif

static const char	*flagdelim = ",";

/*
 * Convert the character representation of audit values into the au_mask_t
 * field.
 */
int
getauditflagsbin(char *auditstr, au_mask_t *masks)
{
	char class_ent_name[AU_CLASS_NAME_MAX];
	char class_ent_desc[AU_CLASS_DESC_MAX];
	struct au_class_ent c;
	char *tok;
	char sel, sub;
	char *last;

	bzero(&c, sizeof(c));
	bzero(class_ent_name, sizeof(class_ent_name));
	bzero(class_ent_desc, sizeof(class_ent_desc));
	c.ac_name = class_ent_name;
	c.ac_desc = class_ent_desc;

	masks->am_success = 0;
	masks->am_failure = 0;

	tok = strtok_r(auditstr, flagdelim, &last);
	while (tok != NULL) {
		/* Check for the events that should not be audited. */
		if (tok[0] == '^') {
			sub = 1;
			tok++;
		} else
			sub = 0;

		/* Check for the events to be audited for success. */
		if (tok[0] == '+') {
			sel = AU_PRS_SUCCESS;
			tok++;
		} else if (tok[0] == '-') {
			sel = AU_PRS_FAILURE;
			tok++;
		} else
			sel = AU_PRS_BOTH;

		if ((getauclassnam_r(&c, tok)) != NULL) {
			if (sub)
				SUB_FROM_MASK(masks, c.ac_class, sel);
			else
				ADD_TO_MASK(masks, c.ac_class, sel);
		} else {
			errno = EINVAL;
			return (-1);
		}

		/* Get the next class. */
		tok = strtok_r(NULL, flagdelim, &last);
	}
	return (0);
}

/*
 * Convert the au_mask_t fields into a string value.  If verbose is non-zero
 * the long flag names are used else the short (2-character)flag names are
 * used.
 *
 * XXXRW: If bits are specified that are not matched by any class, they are
 * omitted rather than rejected with EINVAL.
 *
 * XXXRW: This is not thread-safe as it relies on atomicity between
 * setauclass() and sequential calls to getauclassent().  This could be
 * fixed by iterating through the bitmask fields rather than iterating
 * through the classes.
 */
int
getauditflagschar(char *auditstr, au_mask_t *masks, int verbose)
{
	char class_ent_name[AU_CLASS_NAME_MAX];
	char class_ent_desc[AU_CLASS_DESC_MAX];
	struct au_class_ent c;
	char *strptr = auditstr;
	u_char sel;

	bzero(&c, sizeof(c));
	bzero(class_ent_name, sizeof(class_ent_name));
	bzero(class_ent_desc, sizeof(class_ent_desc));
	c.ac_name = class_ent_name;
	c.ac_desc = class_ent_desc;

	/*
	 * Enumerate the class entries, check if each is selected in either
	 * the success or failure masks.
	 */
	setauclass();
	while ((getauclassent_r(&c)) != NULL) {
		sel = 0;

		/* Dont do anything for class = no. */
		if (c.ac_class == 0)
			continue;

		sel |= ((c.ac_class & masks->am_success) == c.ac_class) ?
		    AU_PRS_SUCCESS : 0;
		sel |= ((c.ac_class & masks->am_failure) == c.ac_class) ?
		    AU_PRS_FAILURE : 0;

		/*
		 * No prefix should be attached if both success and failure
		 * are selected.
		 */
		if ((sel & AU_PRS_BOTH) == 0) {
			if ((sel & AU_PRS_SUCCESS) != 0) {
				*strptr = '+';
				strptr = strptr + 1;
			} else if ((sel & AU_PRS_FAILURE) != 0) {
				*strptr = '-';
				strptr = strptr + 1;
			}
		}

		if (sel != 0) {
			if (verbose) {
				strlcpy(strptr, c.ac_desc, AU_CLASS_DESC_MAX);
				strptr += strlen(c.ac_desc);
			} else {
				strlcpy(strptr, c.ac_name, AU_CLASS_NAME_MAX);
				strptr += strlen(c.ac_name);
			}
			*strptr = ','; /* delimiter */
			strptr = strptr + 1;
		}
	}

	/* Overwrite the last delimiter with the string terminator. */
	if (strptr != auditstr)
		*(strptr-1) = '\0';

	return (0);
}