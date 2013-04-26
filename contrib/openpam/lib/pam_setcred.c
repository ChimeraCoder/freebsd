
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
 * XSSO 6 page 57
 *
 * Modify / delete user credentials for an authentication service
 */

int
pam_setcred(pam_handle_t *pamh,
	int flags)
{
	int r;

	ENTER();
	if (flags & ~(PAM_SILENT|PAM_ESTABLISH_CRED|PAM_DELETE_CRED|
		PAM_REINITIALIZE_CRED|PAM_REFRESH_CRED))
		RETURNC(PAM_SYMBOL_ERR);
	/* XXX enforce exclusivity */
	r = openpam_dispatch(pamh, PAM_SM_SETCRED, flags);
	RETURNC(r);
}

/*
 * Error codes:
 *
 *	=openpam_dispatch
 *	=pam_sm_setcred
 *	!PAM_IGNORE
 *	PAM_SYMBOL_ERR
 */

/**
 * The =pam_setcred function manages the application's credentials.
 *
 * The =flags argument is the binary or of zero or more of the following
 * values:
 *
 *	=PAM_SILENT:
 *		Do not emit any messages.
 *	=PAM_ESTABLISH_CRED:
 *		Establish the credentials of the target user.
 *	=PAM_DELETE_CRED:
 *		Revoke all established credentials.
 *	=PAM_REINITIALIZE_CRED:
 *		Fully reinitialise credentials.
 *	=PAM_REFRESH_CRED:
 *		Refresh credentials.
 *
 * The latter four are mutually exclusive.
 *
 * If any other bits are set, =pam_setcred will return =PAM_SYMBOL_ERR.
 */