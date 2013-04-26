
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

#include <sys/time.h>

#include <paths.h>
#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <utmpx.h>

#define PAM_SM_SESSION

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_mod_misc.h>

#define	PAM_UTMPX_ID	"utmpx_id"

PAM_EXTERN int
pam_sm_open_session(pam_handle_t *pamh, int flags,
    int argc __unused, const char *argv[] __unused)
{
	struct passwd *pwd;
	struct utmpx *utx, utl;
	time_t t;
	const char *user;
	const void *rhost, *tty;
	char *id;
	int pam_err;

	pam_err = pam_get_user(pamh, &user, NULL);
	if (pam_err != PAM_SUCCESS)
		return (pam_err);
	if (user == NULL || (pwd = getpwnam(user)) == NULL)
		return (PAM_SERVICE_ERR);
	PAM_LOG("Got user: %s", user);

	pam_err = pam_get_item(pamh, PAM_RHOST, &rhost);
	if (pam_err != PAM_SUCCESS) {
		PAM_LOG("No PAM_RHOST");
		goto err;
	}
	pam_err = pam_get_item(pamh, PAM_TTY, &tty);
	if (pam_err != PAM_SUCCESS) {
		PAM_LOG("No PAM_TTY");
		goto err;
	}
	if (tty == NULL) {
		PAM_LOG("No PAM_TTY");
		pam_err = PAM_SERVICE_ERR;
		goto err;
	}
	/* Strip /dev/ component. */
	if (strncmp(tty, _PATH_DEV, sizeof(_PATH_DEV) - 1) == 0)
		tty = (const char *)tty + sizeof(_PATH_DEV) - 1;

	if ((flags & PAM_SILENT) == 0) {
		if (setutxdb(UTXDB_LASTLOGIN, NULL) != 0) {
			PAM_LOG("Failed to open lastlogin database");
		} else {
			utx = getutxuser(user);
			if (utx != NULL && utx->ut_type == USER_PROCESS) {
				t = utx->ut_tv.tv_sec;
				if (*utx->ut_host != '\0')
					pam_info(pamh, "Last login: %.*s from %s",
					    24 - 5, ctime(&t), utx->ut_host);
				else
					pam_info(pamh, "Last login: %.*s on %s",
					    24 - 5, ctime(&t), utx->ut_line);
			}
			endutxent();
		}
	}

	id = malloc(sizeof utl.ut_id);
	if (id == NULL) {
		pam_err = PAM_SERVICE_ERR;
		goto err;
	}
	arc4random_buf(id, sizeof utl.ut_id);

	pam_err = pam_set_data(pamh, PAM_UTMPX_ID, id, openpam_free_data);
	if (pam_err != PAM_SUCCESS) {
		free(id);
		goto err;
	}

	memset(&utl, 0, sizeof utl);
	utl.ut_type = USER_PROCESS;
	memcpy(utl.ut_id, id, sizeof utl.ut_id);
	strncpy(utl.ut_user, user, sizeof utl.ut_user);
	strncpy(utl.ut_line, tty, sizeof utl.ut_line);
	if (rhost != NULL)
		strncpy(utl.ut_host, rhost, sizeof utl.ut_host);
	utl.ut_pid = getpid();
	gettimeofday(&utl.ut_tv, NULL);
	pututxline(&utl);

	return (PAM_SUCCESS);

err:
	if (openpam_get_option(pamh, "no_fail"))
		return (PAM_SUCCESS);
	return (pam_err);
}

PAM_EXTERN int
pam_sm_close_session(pam_handle_t *pamh, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{
	struct utmpx utl;
	const void *id;
	int pam_err;

	pam_err = pam_get_data(pamh, PAM_UTMPX_ID, (const void **)&id);
	if (pam_err != PAM_SUCCESS)
		goto err;

	memset(&utl, 0, sizeof utl);
	utl.ut_type = DEAD_PROCESS;
	memcpy(utl.ut_id, id, sizeof utl.ut_id);
	utl.ut_pid = getpid();
	gettimeofday(&utl.ut_tv, NULL);
	pututxline(&utl);

	return (PAM_SUCCESS);

 err:
	if (openpam_get_option(pamh, "no_fail"))
		return (PAM_SUCCESS);
	return (pam_err);
}

PAM_MODULE_ENTRY("pam_lastlog");