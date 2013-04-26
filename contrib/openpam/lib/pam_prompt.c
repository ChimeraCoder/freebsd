
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

#include <stdarg.h>

#include <security/pam_appl.h>
#include <security/openpam.h>

/*
 * OpenPAM extension
 *
 * Call the conversation function
 */

int
pam_prompt(const pam_handle_t *pamh,
	int style,
	char **resp,
	const char *fmt,
	...)
{
	va_list ap;
	int r;

	va_start(ap, fmt);
	r = pam_vprompt(pamh, style, resp, fmt, ap);
	va_end(ap);
	return (r);
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
 * The =pam_prompt function constructs a message from the specified format
 * string and arguments and passes it to the given PAM context's
 * conversation function.
 *
 * A pointer to the response, or =NULL if the conversation function did
 * not return one, is stored in the location pointed to by the =resp
 * argument.
 *
 * See =pam_vprompt for further details.
 *
 * >pam_error
 * >pam_info
 * >pam_vprompt
 */