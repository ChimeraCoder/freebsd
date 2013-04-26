
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

#include <sys/types.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * OpenPAM extension
 *
 * Null conversation function
 */

int
openpam_nullconv(int n,
	 const struct pam_message **msg,
	 struct pam_response **resp,
	 void *data)
{

	ENTER();
	(void)n;
	(void)msg;
	(void)resp;
	(void)data;
	RETURNC(PAM_CONV_ERR);
}

/*
 * Error codes:
 *
 *	PAM_CONV_ERR
 */

/**
 * The =openpam_nullconv function is a null conversation function suitable
 * for applications that want to use PAM but don't support interactive
 * dialog with the user.
 * Such applications should set =PAM_AUTHTOK to whatever authentication
 * token they've obtained on their own before calling =pam_authenticate
 * and / or =pam_chauthtok, and their PAM configuration should specify the
 * ;use_first_pass option for all modules that require access to the
 * authentication token, to make sure they use =PAM_AUTHTOK rather than
 * try to query the user.
 *
 * >openpam_ttyconv
 * >pam_prompt
 * >pam_set_item
 * >pam_vprompt
 */