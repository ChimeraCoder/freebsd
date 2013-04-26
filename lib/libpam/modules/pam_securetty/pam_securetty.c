
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

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <ttyent.h>
#include <string.h>

#define PAM_SM_ACCOUNT

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_mod_misc.h>

#define TTY_PREFIX	"/dev/"

PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t *pamh __unused, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{
	struct passwd *pwd;
	struct ttyent *ty;
	const char *user;
	const void *tty;
	int pam_err;

	pam_err = pam_get_user(pamh, &user, NULL);
	if (pam_err != PAM_SUCCESS)
		return (pam_err);
	if (user == NULL || (pwd = getpwnam(user)) == NULL)
		return (PAM_SERVICE_ERR);

	PAM_LOG("Got user: %s", user);

	/* If the user is not root, secure ttys do not apply */
	if (pwd->pw_uid != 0)
		return (PAM_SUCCESS);

	pam_err = pam_get_item(pamh, PAM_TTY, &tty);
	if (pam_err != PAM_SUCCESS)
		return (pam_err);

	PAM_LOG("Got TTY: %s", (const char *)tty);

	/* Ignore any "/dev/" on the PAM_TTY item */
	if (tty != NULL && strncmp(TTY_PREFIX, tty, sizeof(TTY_PREFIX)) == 0) {
		PAM_LOG("WARNING: PAM_TTY starts with " TTY_PREFIX);
		tty = (const char *)tty + sizeof(TTY_PREFIX) - 1;
	}

	if (tty != NULL && (ty = getttynam(tty)) != NULL &&
	    (ty->ty_status & TTY_SECURE) != 0)
		return (PAM_SUCCESS);

	PAM_VERBOSE_ERROR("Not on secure TTY");
	return (PAM_AUTH_ERR);
}

PAM_MODULE_ENTRY("pam_securetty");