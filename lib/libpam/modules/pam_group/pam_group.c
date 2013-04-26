
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

#include <grp.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#define PAM_SM_AUTH

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/openpam.h>


PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{
	int local, remote;
	const char *group, *user;
	const void *ruser;
	char *const *list;
	struct passwd *pwd;
	struct group *grp;

	/* get target account */
	if (pam_get_user(pamh, &user, NULL) != PAM_SUCCESS ||
	    user == NULL || (pwd = getpwnam(user)) == NULL)
		return (PAM_AUTH_ERR);
	if (pwd->pw_uid != 0 && openpam_get_option(pamh, "root_only"))
		return (PAM_IGNORE);

	/* check local / remote */
	local = openpam_get_option(pamh, "luser") ? 1 : 0;
	remote = openpam_get_option(pamh, "ruser") ? 1 : 0;
	if (local && remote) {
		openpam_log(PAM_LOG_ERROR, "(pam_group) "
		    "the luser and ruser options are mutually exclusive");
		return (PAM_SERVICE_ERR);
	} else if (local) {
		/* we already have the correct struct passwd */
	} else {
		if (!remote)
			openpam_log(PAM_LOG_NOTICE, "(pam_group) "
			    "neither luser nor ruser specified, assuming ruser");
		/* default / historical behavior */
		if (pam_get_item(pamh, PAM_RUSER, &ruser) != PAM_SUCCESS ||
		    ruser == NULL || (pwd = getpwnam(ruser)) == NULL)
			return (PAM_AUTH_ERR);
	}

	/* get regulating group */
	if ((group = openpam_get_option(pamh, "group")) == NULL)
		group = "wheel";
	if ((grp = getgrnam(group)) == NULL || grp->gr_mem == NULL)
		goto failed;

	/* check if the group is empty */
	if (*grp->gr_mem == NULL)
		goto failed;

	/* check membership */
	if (pwd->pw_gid == grp->gr_gid)
		goto found;
	for (list = grp->gr_mem; *list != NULL; ++list)
		if (strcmp(*list, pwd->pw_name) == 0)
			goto found;

 not_found:
	if (openpam_get_option(pamh, "deny"))
		return (PAM_SUCCESS);
	return (PAM_AUTH_ERR);
 found:
	if (openpam_get_option(pamh, "deny"))
		return (PAM_AUTH_ERR);
	return (PAM_SUCCESS);
 failed:
	if (openpam_get_option(pamh, "fail_safe"))
		goto found;
	else
		goto not_found;
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t * pamh __unused, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{

	return (PAM_SUCCESS);
}

PAM_MODULE_ENTRY("pam_group");