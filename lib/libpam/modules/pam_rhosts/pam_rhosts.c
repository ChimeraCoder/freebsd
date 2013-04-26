
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

#include <pwd.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#define PAM_SM_AUTH
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_mod_misc.h>

#define OPT_ALLOW_ROOT "allow_root"

PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{
	struct passwd *pw;
	const char *user;
	const void *ruser, *rhost;
	int err, superuser;

	err = pam_get_user(pamh, &user, NULL);
	if (err != PAM_SUCCESS)
		return (err);

	if ((pw = getpwnam(user)) == NULL)
		return (PAM_USER_UNKNOWN);
	if (pw->pw_uid == 0 &&
	    openpam_get_option(pamh, OPT_ALLOW_ROOT) == NULL)
		return (PAM_AUTH_ERR);

	err = pam_get_item(pamh, PAM_RUSER, &ruser);
	if (err != PAM_SUCCESS)
		return (PAM_AUTH_ERR);

	err = pam_get_item(pamh, PAM_RHOST, &rhost);
	if (err != PAM_SUCCESS)
		return (PAM_AUTH_ERR);

	superuser = (strcmp(user, "root") == 0);
	err = ruserok(rhost, superuser, ruser, user);
	if (err != 0)
		return (PAM_AUTH_ERR);

	return (PAM_SUCCESS);
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t *pamh __unused, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{

	return (PAM_SUCCESS);
}

PAM_MODULE_ENTRY("pam_rhosts");