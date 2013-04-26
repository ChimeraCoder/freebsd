
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

#include <ctype.h>
#include <grp.h>
#include <paths.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAM_SM_ACCOUNT

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_mod_misc.h>
#include <security/openpam.h>

PAM_EXTERN int
pam_sm_acct_mgmt(pam_handle_t *pamh, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{
	struct passwd *pwd;
	struct group *grp;
	const char *user;
	int pam_err, found, allow;
	char *line, *name, **mem;
	size_t len, ulen;
	FILE *f;

	pam_err = pam_get_user(pamh, &user, NULL);
	if (pam_err != PAM_SUCCESS)
		return (pam_err);
	if (user == NULL || (pwd = getpwnam(user)) == NULL)
		return (PAM_SERVICE_ERR);

	found = 0;
	ulen = strlen(user);
	if ((f = fopen(_PATH_FTPUSERS, "r")) == NULL) {
		PAM_LOG("%s: %m", _PATH_FTPUSERS);
		goto done;
	}
	while (!found && (line = fgetln(f, &len)) != NULL) {
		if (*line == '#')
			continue;
		while (len > 0 && isspace(line[len - 1]))
			--len;
		if (len == 0)
			continue;
		/* simple case first */
		if (*line != '@') {
			if (len == ulen && strncmp(user, line, len) == 0)
				found = 1;
			continue;
		}
		/* member of specified group? */
		asprintf(&name, "%.*s", (int)len - 1, line + 1);
		if (name == NULL) {
			fclose(f);
			return (PAM_BUF_ERR);
		}
		grp = getgrnam(name);
		free(name);
		if (grp == NULL)
			continue;
		for (mem = grp->gr_mem; mem && *mem && !found; ++mem)
			if (strcmp(user, *mem) == 0)
				found = 1;
	}
 done:
	allow = (openpam_get_option(pamh, "disallow") == NULL);
	if (found)
		pam_err = allow ? PAM_SUCCESS : PAM_AUTH_ERR;
	else
		pam_err = allow ? PAM_AUTH_ERR : PAM_SUCCESS;
	if (f != NULL)
		fclose(f);
	return (pam_err);
}

PAM_MODULE_ENTRY("pam_ftpusers");