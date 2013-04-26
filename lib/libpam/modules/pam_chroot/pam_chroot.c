
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

#include <sys/param.h>

#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PAM_SM_SESSION

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/openpam.h>

PAM_EXTERN int
pam_sm_open_session(pam_handle_t *pamh, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{
	const char *dir, *end, *cwd, *user;
	struct passwd *pwd;
	char buf[PATH_MAX];

	if (pam_get_user(pamh, &user, NULL) != PAM_SUCCESS ||
	    user == NULL || (pwd = getpwnam(user)) == NULL)
		return (PAM_SESSION_ERR);
	if (pwd->pw_uid == 0 && !openpam_get_option(pamh, "also_root"))
		return (PAM_SUCCESS);
	if (pwd->pw_dir == NULL)
		return (PAM_SESSION_ERR);
	if ((end = strstr(pwd->pw_dir, "/./")) != NULL) {
		if (snprintf(buf, sizeof(buf), "%.*s",
		    (int)(end - pwd->pw_dir), pwd->pw_dir) > (int)sizeof(buf)) {
			openpam_log(PAM_LOG_ERROR,
			    "%s's home directory is too long", user);
			return (PAM_SESSION_ERR);
		}
		dir = buf;
		cwd = end + 2;
	} else if ((dir = openpam_get_option(pamh, "dir")) != NULL) {
		if ((cwd = openpam_get_option(pamh, "cwd")) == NULL)
			cwd = "/";
	} else {
		if (openpam_get_option(pamh, "always")) {
			openpam_log(PAM_LOG_ERROR,
			    "%s has no chroot directory", user);
			return (PAM_SESSION_ERR);
		}
		return (PAM_SUCCESS);
	}

	openpam_log(PAM_LOG_DEBUG, "chrooting %s to %s", dir, user);

	if (chroot(dir) == -1) {
		openpam_log(PAM_LOG_ERROR, "chroot(): %m");
		return (PAM_SESSION_ERR);
	}
	if (chdir(cwd) == -1) {
		openpam_log(PAM_LOG_ERROR, "chdir(): %m");
		return (PAM_SESSION_ERR);
	}
	pam_setenv(pamh, "HOME", cwd, 1);
	return (PAM_SUCCESS);
}

PAM_EXTERN int
pam_sm_close_session(pam_handle_t *pamh __unused, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{

	return (PAM_SUCCESS);
}

PAM_MODULE_ENTRY("pam_chroot");