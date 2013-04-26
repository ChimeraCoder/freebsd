
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
#include <security/pam_modules.h>

/*
 * XSSO 4.2.2
 * XSSO 6 page 66
 *
 * Service module implementation for pam_acct_mgmt
 */

int
pam_sm_acct_mgmt(pam_handle_t *pamh,
	int flags,
	int argc,
	const char **argv)
{

	ENTER();
	RETURNC(PAM_SYSTEM_ERR);
}

/*
 * Error codes:
 *
 *	PAM_SERVICE_ERR
 *	PAM_SYSTEM_ERR
 *	PAM_BUF_ERR
 *	PAM_CONV_ERR
 *	PAM_PERM_DENIED
 *	PAM_IGNORE
 *	PAM_ABORT
 *
 *	PAM_USER_UNKNOWN
 *	PAM_AUTH_ERR
 *	PAM_NEW_AUTHTOK_REQD
 *	PAM_ACCT_EXPIRED
 */

/**
 * The =pam_sm_acct_mgmt function is the service module's implementation
 * of the =pam_acct_mgmt API function.
 */