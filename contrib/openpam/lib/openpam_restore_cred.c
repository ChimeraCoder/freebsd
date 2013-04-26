
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
 * Restore credentials
 */

int
openpam_restore_cred(pam_handle_t *pamh)
{
	const struct pam_saved_cred *scred;
	const void *scredp;
	int r;

	ENTER();
	r = pam_get_data(pamh, PAM_SAVED_CRED, &scredp);
	if (r != PAM_SUCCESS)
		RETURNC(r);
	if (scredp == NULL)
		RETURNC(PAM_SYSTEM_ERR);
	scred = scredp;
	if (scred->euid != geteuid()) {
		if (seteuid(scred->euid) < 0 ||
		    setgroups(scred->ngroups, scred->groups) < 0 ||
		    setegid(scred->egid) < 0)
			RETURNC(PAM_SYSTEM_ERR);
	}
	pam_set_data(pamh, PAM_SAVED_CRED, NULL, NULL);
	RETURNC(PAM_SUCCESS);
}

/*
 * Error codes:
 *
 *	=pam_get_data
 *	PAM_SYSTEM_ERR
 */

/**
 * The =openpam_restore_cred function restores the credentials saved by
 * =openpam_borrow_cred.
 *
 * >setegid 2
 * >seteuid 2
 * >setgroups 2
 */