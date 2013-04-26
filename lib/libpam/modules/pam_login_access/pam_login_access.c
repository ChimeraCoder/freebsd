
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

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#define _BSD_SOURCE

#include <sys/param.h>

#include <syslog.h>
#include <unistd.h>

#define PAM_SM_ACCOUNT

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_mod_misc.h>

#include "pam_login_access.h"

PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t *pamh, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{
	const void *rhost, *tty, *user;
	char hostname[MAXHOSTNAMELEN];
	int pam_err;

	pam_err = pam_get_item(pamh, PAM_USER, &user);
	if (pam_err != PAM_SUCCESS)
		return (pam_err);

	if (user == NULL)
		return (PAM_SERVICE_ERR);

	PAM_LOG("Got user: %s", (const char *)user);

	pam_err = pam_get_item(pamh, PAM_RHOST, &rhost);
	if (pam_err != PAM_SUCCESS)
		return (pam_err);

	pam_err = pam_get_item(pamh, PAM_TTY, &tty);
	if (pam_err != PAM_SUCCESS)
		return (pam_err);

	gethostname(hostname, sizeof hostname);

	if (rhost == NULL || *(const char *)rhost == '\0') {
		PAM_LOG("Checking login.access for user %s on tty %s",
		    (const char *)user, (const char *)tty);
		if (login_access(user, tty) != 0)
			return (PAM_SUCCESS);
		PAM_VERBOSE_ERROR("%s is not allowed to log in on %s",
		    user, tty);
	} else {
		PAM_LOG("Checking login.access for user %s from host %s",
		    (const char *)user, (const char *)rhost);
		if (login_access(user, rhost) != 0)
			return (PAM_SUCCESS);
		PAM_VERBOSE_ERROR("%s is not allowed to log in from %s",
		    user, rhost);
	}

	return (PAM_AUTH_ERR);
}

PAM_MODULE_ENTRY("pam_login_access");