
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

#include <string.h>

#define PAM_SM_AUTH

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/openpam.h>

#define DEFAULT_GUESTS	"guest"

static int
lookup(const char *str, const char *list)
{
	const char *next;
	size_t len;

	len = strlen(str);
	while (*list != '\0') {
		while (*list == ',')
			++list;
		if ((next = strchr(list, ',')) == NULL)
			next = strchr(list, '\0');
		if (next - list == (ptrdiff_t)len &&
		    strncmp(list, str, len) == 0)
			return (1);
		list = next;
	}
	return (0);
}

PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{
	const char *authtok, *guests, *user;
	int err, is_guest;

	/* get target account */
	if (pam_get_user(pamh, &user, NULL) != PAM_SUCCESS || user == NULL)
		return (PAM_AUTH_ERR);

	/* get list of guest logins */
	if ((guests = openpam_get_option(pamh, "guests")) == NULL)
		guests = DEFAULT_GUESTS;

	/* check if the target account is on the list */
	is_guest = lookup(user, guests);

	/* check password */
	if (!openpam_get_option(pamh, "nopass")) {
		err = pam_get_authtok(pamh, PAM_AUTHTOK, &authtok, NULL);
		if (err != PAM_SUCCESS)
			return (err);
		if (openpam_get_option(pamh, "pass_is_user") &&
		    strcmp(user, authtok) != 0)
			return (PAM_AUTH_ERR);
		if (openpam_get_option(pamh, "pass_as_ruser"))
			pam_set_item(pamh, PAM_RUSER, authtok);
	}

	/* done */
	if (is_guest) {
		pam_setenv(pamh, "GUEST", user, 1);
		return (PAM_SUCCESS);
	}
	return (PAM_AUTH_ERR);
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t * pamh __unused, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{

	return (PAM_SUCCESS);
}

PAM_MODULE_ENTRY("pam_guest");