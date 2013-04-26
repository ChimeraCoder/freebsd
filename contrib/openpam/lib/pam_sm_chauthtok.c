
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
 * XSSO 6 page 72
 *
 * Service module implementation for pam_chauthtok
 */

int
pam_sm_chauthtok(pam_handle_t *pamh,
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
 *	PAM_PERM_DENIED
 *	PAM_AUTHTOK_ERR
 *	PAM_AUTHTOK_RECOVERY_ERR
 *	PAM_AUTHTOK_LOCK_BUSY
 *	PAM_AUTHTOK_DISABLE_AGING
 *	PAM_TRY_AGAIN
 */

/**
 * The =pam_sm_chauthtok function is the service module's implementation
 * of the =pam_chauthtok API function.
 *
 * When the application calls =pam_chauthtok, the service function is
 * called twice, first with the =PAM_PRELIM_CHECK flag set and then again
 * with the =PAM_UPDATE_AUTHTOK flag set.
 */