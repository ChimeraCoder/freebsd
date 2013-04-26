
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
#include <fcntl.h>
#include <login_cap.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PAM_SM_ACCOUNT

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_mod_misc.h>

#define	_PATH_NOLOGIN	"/var/run/nologin"

static char nologin_def[] = _PATH_NOLOGIN;

PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t *pamh, int flags,
    int argc __unused, const char *argv[] __unused)
{
	login_cap_t *lc;
	struct passwd *pwd;
	struct stat st;
	int retval, fd;
	ssize_t ss;
	const char *user, *nologin;
	char *mtmp;

	retval = pam_get_user(pamh, &user, NULL);
	if (retval != PAM_SUCCESS)
		return (retval);

	PAM_LOG("Got user: %s", user);

	pwd = getpwnam(user);
	if (pwd == NULL)
		return (PAM_USER_UNKNOWN);

	/*
	 * login_getpwclass(3) will select the "root" class by default
	 * if pwd->pw_uid is 0.  That class should have "ignorenologin"
	 * capability so that super-user can bypass nologin.
	 */
	lc = login_getpwclass(pwd);
	if (lc == NULL) {
		PAM_LOG("Unable to get login class for user %s", user);
		return (PAM_SERVICE_ERR);
	}

	if (login_getcapbool(lc, "ignorenologin", 0)) {
		login_close(lc);
		return (PAM_SUCCESS);
	}

	nologin = login_getcapstr(lc, "nologin", nologin_def, nologin_def);

	fd = open(nologin, O_RDONLY, 0);
	if (fd < 0) {
		login_close(lc);
		return (PAM_SUCCESS);
	}

	PAM_LOG("Opened %s file", nologin);

	if (fstat(fd, &st) == 0) {
		mtmp = malloc(st.st_size + 1);
		if (mtmp != NULL) {
			ss = read(fd, mtmp, st.st_size);
			if (ss > 0) {
				mtmp[ss] = '\0';
				pam_error(pamh, "%s", mtmp);
			}
			free(mtmp);
		}
	}

	PAM_VERBOSE_ERROR("Administrator refusing you: %s", nologin);

	close(fd);
	login_close(lc);

	return (PAM_AUTH_ERR);
}

PAM_MODULE_ENTRY("pam_nologin");