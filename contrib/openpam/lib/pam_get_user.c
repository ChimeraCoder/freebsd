
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

#include <stdlib.h>

#include <security/pam_appl.h>
#include <security/openpam.h>

#include "openpam_impl.h"

static const char user_prompt[] = "Login:";

/*
 * XSSO 4.2.1
 * XSSO 6 page 52
 *
 * Retrieve user name
 */

int
pam_get_user(pam_handle_t *pamh,
	const char **user,
	const char *prompt)
{
	char prompt_buf[1024];
	size_t prompt_size;
	const void *promptp;
	char *resp;
	int r;

	ENTER();
	if (pamh == NULL || user == NULL)
		RETURNC(PAM_SYSTEM_ERR);
	r = pam_get_item(pamh, PAM_USER, (const void **)user);
	if (r == PAM_SUCCESS && *user != NULL)
		RETURNC(PAM_SUCCESS);
	/* pam policy overrides the module's choice */
	if ((promptp = openpam_get_option(pamh, "user_prompt")) != NULL)
		prompt = promptp;
	/* no prompt provided, see if there is one tucked away somewhere */
	if (prompt == NULL)
		if (pam_get_item(pamh, PAM_USER_PROMPT, &promptp) &&
		    promptp != NULL)
			prompt = promptp;
	/* fall back to hardcoded default */
	if (prompt == NULL)
		prompt = user_prompt;
	/* expand */
	prompt_size = sizeof prompt_buf;
	r = openpam_subst(pamh, prompt_buf, &prompt_size, prompt);
	if (r == PAM_SUCCESS && prompt_size <= sizeof prompt_buf)
		prompt = prompt_buf;
	r = pam_prompt(pamh, PAM_PROMPT_ECHO_ON, &resp, "%s", prompt);
	if (r != PAM_SUCCESS)
		RETURNC(r);
	r = pam_set_item(pamh, PAM_USER, resp);
	FREE(resp);
	if (r != PAM_SUCCESS)
		RETURNC(r);
	r = pam_get_item(pamh, PAM_USER, (const void **)user);
	RETURNC(r);
}

/*
 * Error codes:
 *
 *	=pam_get_item
 *	=pam_prompt
 *	=pam_set_item
 *	!PAM_SYMBOL_ERR
 */

/**
 * The =pam_get_user function returns the name of the target user, as
 * specified to =pam_start.
 * If no user was specified, nor set using =pam_set_item, =pam_get_user
 * will prompt for a user name.
 * Either way, a pointer to the user name is stored in the location
 * pointed to by the =user argument.
 *
 * The =prompt argument specifies a prompt to use if no user name is
 * cached.
 * If it is =NULL, the =PAM_USER_PROMPT item will be used.
 * If that item is also =NULL, a hardcoded default prompt will be used.
 * Either way, the prompt is expanded using =openpam_subst before it is
 * passed to the conversation function.
 *
 * If =pam_get_user is called from a module and the ;user_prompt option is
 * set in the policy file, the value of that option takes precedence over
 * both the =prompt argument and the =PAM_USER_PROMPT item.
 *
 * >pam_get_item
 * >pam_get_authtok
 * >openpam_subst
 */