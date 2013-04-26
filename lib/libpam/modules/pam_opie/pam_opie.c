
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
#include <opie.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PAM_SM_AUTH

#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_mod_misc.h>

#define PAM_OPT_NO_FAKE_PROMPTS	"no_fake_prompts"

PAM_EXTERN int
pam_sm_authenticate(pam_handle_t *pamh, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{
	struct opie opie;
	struct passwd *pwd;
	int retval, i;
	const char *(promptstr[]) = { "%s\nPassword: ", "%s\nPassword [echo on]: "};
	char challenge[OPIE_CHALLENGE_MAX];
	char principal[OPIE_PRINCIPAL_MAX];
	const char *user;
	char *response;
	int style;

	user = NULL;
	if (openpam_get_option(pamh, PAM_OPT_AUTH_AS_SELF)) {
		if ((pwd = getpwnam(getlogin())) == NULL)
			return (PAM_AUTH_ERR);
		user = pwd->pw_name;
	}
	else {
		retval = pam_get_user(pamh, &user, NULL);
		if (retval != PAM_SUCCESS)
			return (retval);
	}

	PAM_LOG("Got user: %s", user);

	/*
	 * Watch out: libopie feels entitled to truncate the user name
	 * passed to it if it's longer than OPIE_PRINCIPAL_MAX, which is
	 * not uncommon in Windows environments.
	 */
	if (strlen(user) >= sizeof(principal))
		return (PAM_AUTH_ERR);
	strlcpy(principal, user, sizeof(principal));

	/*
	 * Don't call the OPIE atexit() handler when our program exits,
	 * since the module has been unloaded and we will SEGV.
	 */
	opiedisableaeh();

	/*
	 * If the no_fake_prompts option was given, and the user
	 * doesn't have an OPIE key, just fail rather than present the
	 * user with a bogus OPIE challenge.
	 */
	if (opiechallenge(&opie, principal, challenge) != 0 &&
	    openpam_get_option(pamh, PAM_OPT_NO_FAKE_PROMPTS))
		return (PAM_AUTH_ERR);

	/*
	 * It doesn't make sense to use a password that has already been
	 * typed in, since we haven't presented the challenge to the user
	 * yet, so clear the stored password.
	 */
	pam_set_item(pamh, PAM_AUTHTOK, NULL);

	style = PAM_PROMPT_ECHO_OFF;
	for (i = 0; i < 2; i++) {
		retval = pam_prompt(pamh, style, &response,
		    promptstr[i], challenge);
		if (retval != PAM_SUCCESS) {
			opieunlock();
			return (retval);
		}

		PAM_LOG("Completed challenge %d: %s", i, response);

		if (response[0] != '\0')
			break;

		/* Second time round, echo the password */
		style = PAM_PROMPT_ECHO_ON;
	}

	pam_set_item(pamh, PAM_AUTHTOK, response);

	/*
	 * Opieverify is supposed to return -1 only if an error occurs.
	 * But it returns -1 even if the response string isn't in the form
	 * it expects.  Thus we can't log an error and can only check for
	 * success or lack thereof.
	 */
	retval = opieverify(&opie, response);
	free(response);
	return (retval == 0 ? PAM_SUCCESS : PAM_AUTH_ERR);
}

PAM_EXTERN int
pam_sm_setcred(pam_handle_t *pamh __unused, int flags __unused,
    int argc __unused, const char *argv[] __unused)
{

	return (PAM_SUCCESS);
}

PAM_MODULE_ENTRY("pam_opie");