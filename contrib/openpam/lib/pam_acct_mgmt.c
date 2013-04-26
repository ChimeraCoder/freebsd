
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
 * XSSO 6 page 32
 *
 * Perform PAM account validation procedures
 */

int
pam_acct_mgmt(pam_handle_t *pamh,
	int flags)
{
	int r;

	ENTER();
	r = openpam_dispatch(pamh, PAM_SM_ACCT_MGMT, flags);
	RETURNC(r);
}

/*
 * Error codes:
 *
 *	=openpam_dispatch
 *	=pam_sm_acct_mgmt
 *	!PAM_IGNORE
 */

/**
 * The =pam_acct_mgmt function verifies and enforces account restrictions
 * after the user has been authenticated.
 *
 * The =flags argument is the binary or of zero or more of the following
 * values:
 *
 *	=PAM_SILENT:
 *		Do not emit any messages.
 *	=PAM_DISALLOW_NULL_AUTHTOK:
 *		Fail if the user's authentication token is null.
 *
 * If any other bits are set, =pam_acct_mgmt will return
 * =PAM_SYMBOL_ERR.
 */