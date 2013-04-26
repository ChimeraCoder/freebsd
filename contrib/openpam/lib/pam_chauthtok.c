
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
 * XSSO 6 page 38
 *
 * Perform password related functions within the PAM framework
 */

int
pam_chauthtok(pam_handle_t *pamh,
	int flags)
{
	int r;

	ENTER();
	if (flags & ~(PAM_SILENT|PAM_CHANGE_EXPIRED_AUTHTOK))
		RETURNC(PAM_SYMBOL_ERR);
	r = openpam_dispatch(pamh, PAM_SM_CHAUTHTOK,
	    flags | PAM_PRELIM_CHECK);
	if (r == PAM_SUCCESS)
		r = openpam_dispatch(pamh, PAM_SM_CHAUTHTOK,
		    flags | PAM_UPDATE_AUTHTOK);
	pam_set_item(pamh, PAM_OLDAUTHTOK, NULL);
	pam_set_item(pamh, PAM_AUTHTOK, NULL);
	RETURNC(r);
}

/*
 * Error codes:
 *
 *	=openpam_dispatch
 *	=pam_sm_chauthtok
 *	!PAM_IGNORE
 *	PAM_SYMBOL_ERR
 */

/**
 * The =pam_chauthtok function attempts to change the authentication token
 * for the user associated with the pam context specified by the =pamh
 * argument.
 *
 * The =flags argument is the binary or of zero or more of the following
 * values:
 *
 *	=PAM_SILENT:
 *		Do not emit any messages.
 *	=PAM_CHANGE_EXPIRED_AUTHTOK:
 *		Change only those authentication tokens that have expired.
 *
 * If any other bits are set, =pam_chauthtok will return =PAM_SYMBOL_ERR.
 */