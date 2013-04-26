
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

#include <security/pam_appl.h>

#include "openpam_impl.h"

const char *pam_err_name[PAM_NUM_ERRORS] = {
	"PAM_SUCCESS",
	"PAM_OPEN_ERR",
	"PAM_SYMBOL_ERR",
	"PAM_SERVICE_ERR",
	"PAM_SYSTEM_ERR",
	"PAM_BUF_ERR",
	"PAM_CONV_ERR",
	"PAM_PERM_DENIED",
	"PAM_MAXTRIES",
	"PAM_AUTH_ERR",
	"PAM_NEW_AUTHTOK_REQD",
	"PAM_CRED_INSUFFICIENT",
	"PAM_AUTHINFO_UNAVAIL",
	"PAM_USER_UNKNOWN",
	"PAM_CRED_UNAVAIL",
	"PAM_CRED_EXPIRED",
	"PAM_CRED_ERR",
	"PAM_ACCT_EXPIRED",
	"PAM_AUTHTOK_EXPIRED",
	"PAM_SESSION_ERR",
	"PAM_AUTHTOK_ERR",
	"PAM_AUTHTOK_RECOVERY_ERR",
	"PAM_AUTHTOK_LOCK_BUSY",
	"PAM_AUTHTOK_DISABLE_AGING",
	"PAM_NO_MODULE_DATA",
	"PAM_IGNORE",
	"PAM_ABORT",
	"PAM_TRY_AGAIN",
	"PAM_MODULE_UNKNOWN",
	"PAM_DOMAIN_UNKNOWN"
};

const char *pam_item_name[PAM_NUM_ITEMS] = {
	"(NO ITEM)",
	"PAM_SERVICE",
	"PAM_USER",
	"PAM_TTY",
	"PAM_RHOST",
	"PAM_CONV",
	"PAM_AUTHTOK",
	"PAM_OLDAUTHTOK",
	"PAM_RUSER",
	"PAM_USER_PROMPT",
	"PAM_REPOSITORY",
	"PAM_AUTHTOK_PROMPT",
	"PAM_OLDAUTHTOK_PROMPT",
	"PAM_HOST",
};

const char *pam_facility_name[PAM_NUM_FACILITIES] = {
	[PAM_ACCOUNT]		= "account",
	[PAM_AUTH]		= "auth",
	[PAM_PASSWORD]		= "password",
	[PAM_SESSION]		= "session",
};

const char *pam_control_flag_name[PAM_NUM_CONTROL_FLAGS] = {
	[PAM_BINDING]		= "binding",
	[PAM_OPTIONAL]		= "optional",
	[PAM_REQUIRED]		= "required",
	[PAM_REQUISITE]		= "requisite",
	[PAM_SUFFICIENT]	= "sufficient",
};

const char *pam_func_name[PAM_NUM_PRIMITIVES] = {
	"pam_authenticate",
	"pam_setcred",
	"pam_acct_mgmt",
	"pam_open_session",
	"pam_close_session",
	"pam_chauthtok"
};

const char *pam_sm_func_name[PAM_NUM_PRIMITIVES] = {
	"pam_sm_authenticate",
	"pam_sm_setcred",
	"pam_sm_acct_mgmt",
	"pam_sm_open_session",
	"pam_sm_close_session",
	"pam_sm_chauthtok"
};