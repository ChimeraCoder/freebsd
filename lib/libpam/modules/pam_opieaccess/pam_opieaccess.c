
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

#include <sys/types.h>
#include <opie.h>
#include <pwd.h>
#include <unistd.h>
#include <syslog.h>

#define PAM_SM_AUTH

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_mod_misc.h>

PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{
	struct opie opie;
	struct passwd *pwent;
	const void *luser, *rhost;
	int r;

	r = pam_get_item(pamh, PAM_USER, &luser);
	if (r != PAM_SUCCESS)
		return (r);
	if (luser == NULL)
		return (PAM_SERVICE_ERR);

	pwent = getpwnam(luser);
	if (pwent == NULL || opielookup(&opie, __DECONST(char *, luser)) != 0)
		return (PAM_SUCCESS);

	r = pam_get_item(pamh, PAM_RHOST, &rhost);
	if (r != PAM_SUCCESS)
		return (r);
	if (rhost == NULL || *(const char *)rhost == '\0')
		rhost = openpam_get_option(pamh, "allow_local") ?
		    "" : "localhost";

	if (opieaccessfile(__DECONST(char *, rhost)) != 0 &&
	    opiealways(pwent->pw_dir) != 0)
		return (PAM_SUCCESS);

	PAM_VERBOSE_ERROR("Refused; remote host is not in opieaccess");

	return (PAM_AUTH_ERR);
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t *pamh __unused, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{

	return (PAM_SUCCESS);
}

PAM_MODULE_ENTRY("pam_opieaccess");