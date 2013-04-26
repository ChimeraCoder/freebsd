
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

#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * OpenPAM extension
 *
 * Temporarily borrow user credentials
 */

int
openpam_borrow_cred(pam_handle_t *pamh,
	const struct passwd *pwd)
{
	struct pam_saved_cred *scred;
	const void *scredp;
	int r;

	ENTERI(pwd->pw_uid);
	r = pam_get_data(pamh, PAM_SAVED_CRED, &scredp);
	if (r == PAM_SUCCESS && scredp != NULL) {
		openpam_log(PAM_LOG_DEBUG,
		    "already operating under borrowed credentials");
		RETURNC(PAM_SYSTEM_ERR);
	}
	if (geteuid() != 0 && geteuid() != pwd->pw_uid) {
		openpam_log(PAM_LOG_DEBUG, "called with non-zero euid: %d",
		    (int)geteuid());
		RETURNC(PAM_PERM_DENIED);
	}
	scred = calloc(1, sizeof *scred);
	if (scred == NULL)
		RETURNC(PAM_BUF_ERR);
	scred->euid = geteuid();
	scred->egid = getegid();
	r = getgroups(NGROUPS_MAX, scred->groups);
	if (r < 0) {
		FREE(scred);
		RETURNC(PAM_SYSTEM_ERR);
	}
	scred->ngroups = r;
	r = pam_set_data(pamh, PAM_SAVED_CRED, scred, &openpam_free_data);
	if (r != PAM_SUCCESS) {
		FREE(scred);
		RETURNC(r);
	}
	if (geteuid() == pwd->pw_uid)
		RETURNC(PAM_SUCCESS);
	if (initgroups(pwd->pw_name, pwd->pw_gid) < 0 ||
	      setegid(pwd->pw_gid) < 0 || seteuid(pwd->pw_uid) < 0) {
		openpam_restore_cred(pamh);
		RETURNC(PAM_SYSTEM_ERR);
	}
	RETURNC(PAM_SUCCESS);
}

/*
 * Error codes:
 *
 *	=pam_set_data
 *	PAM_SYSTEM_ERR
 *	PAM_BUF_ERR
 *	PAM_PERM_DENIED
 */

/**
 * The =openpam_borrow_cred function saves the current credentials and
 * switches to those of the user specified by its =pwd argument.
 * The affected credentials are the effective UID, the effective GID, and
 * the group access list.
 * The original credentials can be restored using =openpam_restore_cred.
 *
 * >setegid 2
 * >seteuid 2
 * >setgroups 2
 */