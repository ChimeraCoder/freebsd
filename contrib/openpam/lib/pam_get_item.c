
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <sys/param.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * XSSO 4.2.1
 * XSSO 6 page 46
 *
 * Get PAM information
 */

int
pam_get_item(const pam_handle_t *pamh,
	int item_type,
	const void **item)
{

	ENTERI(item_type);
	if (pamh == NULL)
		RETURNC(PAM_SYSTEM_ERR);
	switch (item_type) {
	case PAM_SERVICE:
	case PAM_USER:
	case PAM_AUTHTOK:
	case PAM_OLDAUTHTOK:
	case PAM_TTY:
	case PAM_RHOST:
	case PAM_RUSER:
	case PAM_CONV:
	case PAM_USER_PROMPT:
	case PAM_REPOSITORY:
	case PAM_AUTHTOK_PROMPT:
	case PAM_OLDAUTHTOK_PROMPT:
	case PAM_HOST:
		*item = pamh->item[item_type];
		RETURNC(PAM_SUCCESS);
	default:
		RETURNC(PAM_SYMBOL_ERR);
	}
}

/*
 * Error codes:
 *
 *	PAM_SYMBOL_ERR
 *	PAM_SYSTEM_ERR
 */

/**
 * The =pam_get_item function stores a pointer to the item specified by
 * the =item_type argument in the location pointed to by the =item
 * argument.
 * The item is retrieved from the PAM context specified by the =pamh
 * argument.
 * If =pam_get_item fails, the =item argument is untouched.
 *
 * The following item types are recognized:
 *
 *	=PAM_SERVICE:
 *		The name of the requesting service.
 *	=PAM_USER:
 *		The name of the user the application is trying to
 *		authenticate.
 *	=PAM_TTY:
 *		The name of the current terminal.
 *	=PAM_RHOST:
 *		The name of the applicant's host.
 *	=PAM_CONV:
 *		A =struct pam_conv describing the current conversation
 *		function.
 *	=PAM_AUTHTOK:
 *		The current authentication token.
 *	=PAM_OLDAUTHTOK:
 *		The expired authentication token.
 *	=PAM_RUSER:
 *		The name of the applicant.
 *	=PAM_USER_PROMPT:
 *		The prompt to use when asking the applicant for a user
 *		name to authenticate as.
 *	=PAM_AUTHTOK_PROMPT:
 *		The prompt to use when asking the applicant for an
 *		authentication token.
 *	=PAM_OLDAUTHTOK_PROMPT:
 *		The prompt to use when asking the applicant for an
 *		expired authentication token prior to changing it.
 *	=PAM_HOST:
 *		The name of the host the application runs on.
 *
 * See =pam_start for a description of =struct pam_conv.
 *
 * >pam_set_item
 */