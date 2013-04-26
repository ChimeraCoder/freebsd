
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

#include <stdio.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * XSSO 4.2.1
 * XSSO 6 page 92
 *
 * Get PAM standard error message string
 */

const char *
pam_strerror(const pam_handle_t *pamh,
	int error_number)
{
	static char unknown[16];

	(void)pamh;

	switch (error_number) {
	case PAM_SUCCESS:
		return ("success");
	case PAM_OPEN_ERR:
		return ("failed to load module");
	case PAM_SYMBOL_ERR:
		return ("invalid symbol");
	case PAM_SERVICE_ERR:
		return ("error in service module");
	case PAM_SYSTEM_ERR:
		return ("system error");
	case PAM_BUF_ERR:
		return ("memory buffer error");
	case PAM_CONV_ERR:
		return ("conversation failure");
	case PAM_PERM_DENIED:
		return ("permission denied");
	case PAM_MAXTRIES:
		return ("maximum number of tries exceeded");
	case PAM_AUTH_ERR:
		return ("authentication error");
	case PAM_NEW_AUTHTOK_REQD:
		return ("new authentication token required");
	case PAM_CRED_INSUFFICIENT:
		return ("insufficient credentials");
	case PAM_AUTHINFO_UNAVAIL:
		return ("authentication information is unavailable");
	case PAM_USER_UNKNOWN:
		return ("unknown user");
	case PAM_CRED_UNAVAIL:
		return ("failed to retrieve user credentials");
	case PAM_CRED_EXPIRED:
		return ("user credentials have expired");
	case PAM_CRED_ERR:
		return ("failed to set user credentials");
	case PAM_ACCT_EXPIRED:
		return ("user account has expired");
	case PAM_AUTHTOK_EXPIRED:
		return ("password has expired");
	case PAM_SESSION_ERR:
		return ("session failure");
	case PAM_AUTHTOK_ERR:
		return ("authentication token failure");
	case PAM_AUTHTOK_RECOVERY_ERR:
		return ("failed to recover old authentication token");
	case PAM_AUTHTOK_LOCK_BUSY:
		return ("authentication token lock busy");
	case PAM_AUTHTOK_DISABLE_AGING:
		return ("authentication token aging disabled");
	case PAM_NO_MODULE_DATA:
		return ("module data not found");
	case PAM_IGNORE:
		return ("ignore this module");
	case PAM_ABORT:
		return ("general failure");
	case PAM_TRY_AGAIN:
		return ("try again");
	case PAM_MODULE_UNKNOWN:
		return ("unknown module type");
	case PAM_DOMAIN_UNKNOWN:
		return ("unknown authentication domain");
	default:
		snprintf(unknown, sizeof unknown, "#%d", error_number);
		return (unknown);
	}
}

/**
 * The =pam_strerror function returns a pointer to a string containing a
 * textual description of the error indicated by the =error_number
 * argument.
 * The =pamh argument is ignored.
 * For compatibility with other implementations, it should be either a
 * valid PAM handle returned by a previous call to =pam_start, or =NULL.
 */