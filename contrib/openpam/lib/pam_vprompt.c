
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * OpenPAM extension
 *
 * Call the conversation function
 */

int
pam_vprompt(const pam_handle_t *pamh,
	int style,
	char **resp,
	const char *fmt,
	va_list ap)
{
	char msgbuf[PAM_MAX_MSG_SIZE];
	struct pam_message msg;
	const struct pam_message *msgp;
	struct pam_response *rsp;
	const struct pam_conv *conv;
	const void *convp;
	int r;

	ENTER();
	r = pam_get_item(pamh, PAM_CONV, &convp);
	if (r != PAM_SUCCESS)
		RETURNC(r);
	conv = convp;
	if (conv == NULL || conv->conv == NULL) {
		openpam_log(PAM_LOG_ERROR, "no conversation function");
		RETURNC(PAM_SYSTEM_ERR);
	}
	vsnprintf(msgbuf, PAM_MAX_MSG_SIZE, fmt, ap);
	msg.msg_style = style;
	msg.msg = msgbuf;
	msgp = &msg;
	rsp = NULL;
	r = (conv->conv)(1, &msgp, &rsp, conv->appdata_ptr);
	*resp = rsp == NULL ? NULL : rsp->resp;
	FREE(rsp);
	RETURNC(r);
}

/*
 * Error codes:
 *
 *     !PAM_SYMBOL_ERR
 *	PAM_SYSTEM_ERR
 *	PAM_BUF_ERR
 *	PAM_CONV_ERR
 */

/**
 * The =pam_vprompt function constructs a string from the =fmt and =ap
 * arguments using =vsnprintf, and passes it to the given PAM context's
 * conversation function.
 *
 * The =style argument specifies the type of interaction requested, and
 * must be one of the following:
 *
 *	=PAM_PROMPT_ECHO_OFF:
 *		Display the message and obtain the user's response without
 *		displaying it.
 *	=PAM_PROMPT_ECHO_ON:
 *		Display the message and obtain the user's response.
 *	=PAM_ERROR_MSG:
 *		Display the message as an error message, and do not wait
 *		for a response.
 *	=PAM_TEXT_INFO:
 *		Display the message as an informational message, and do
 *		not wait for a response.
 *
 * A pointer to the response, or =NULL if the conversation function did
 * not return one, is stored in the location pointed to by the =resp
 * argument.
 *
 * The message and response should not exceed =PAM_MAX_MSG_SIZE or
 * =PAM_MAX_RESP_SIZE, respectively.
 * If they do, they may be truncated.
 *
 * >pam_error
 * >pam_info
 * >pam_prompt
 * >pam_verror
 * >pam_vinfo
 */