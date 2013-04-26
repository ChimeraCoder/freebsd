
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

#include <pwd.h>
#include <unistd.h>
#include <syslog.h>

#define PAM_SM_AUTH

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_mod_misc.h>

#define OPT_ALLOW_ROOT "allow_root"

PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{
	struct passwd *pwd;
	const char *luser;
	int pam_err;
	uid_t uid;

	pam_err = pam_get_user(pamh, &luser, NULL);
	if (pam_err != PAM_SUCCESS)
		return (pam_err);
	if (luser == NULL || (pwd = getpwnam(luser)) == NULL)
		return (PAM_AUTH_ERR);

	uid = getuid();
	if (uid == 0 && !openpam_get_option(pamh, OPT_ALLOW_ROOT))
		return (PAM_AUTH_ERR);

	if (uid == (uid_t)pwd->pw_uid)
		return (PAM_SUCCESS);

	PAM_VERBOSE_ERROR("Refused; source and target users differ");

	return (PAM_AUTH_ERR);
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t *pamh __unused, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{

	return (PAM_SUCCESS);
}

PAM_MODULE_ENTRY("pam_self");