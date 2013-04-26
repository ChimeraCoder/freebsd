
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
 * XSSO 6 page 34
 *
 * Perform authentication within the PAM framework
 */

int
pam_authenticate(pam_handle_t *pamh,
	int flags)
{
	int r;

	ENTER();
	if (flags & ~(PAM_SILENT|PAM_DISALLOW_NULL_AUTHTOK))
		RETURNC(PAM_SYMBOL_ERR);
	r = openpam_dispatch(pamh, PAM_SM_AUTHENTICATE, flags);
	pam_set_item(pamh, PAM_AUTHTOK, NULL);
	RETURNC(r);
}

/*
 * Error codes:
 *
 *	=openpam_dispatch
 *	=pam_sm_authenticate
 *	!PAM_IGNORE
 *	PAM_SYMBOL_ERR
 */

/**
 * The =pam_authenticate function attempts to authenticate the user
 * associated with the pam context specified by the =pamh argument.
 *
 * The application is free to call =pam_authenticate as many times as it
 * wishes, but some modules may maintain an internal retry counter and
 * return =PAM_MAXTRIES when it exceeds some preset or hardcoded limit.
 *
 * The =flags argument is the binary or of zero or more of the following
 * values:
 *
 *	=PAM_SILENT:
 *		Do not emit any messages.
 *	=PAM_DISALLOW_NULL_AUTHTOK:
 *		Fail if the user's authentication token is null.
 *
 * If any other bits are set, =pam_authenticate will return
 * =PAM_SYMBOL_ERR.
 */