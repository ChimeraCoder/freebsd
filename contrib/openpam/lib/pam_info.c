
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
#include <security/openpam.h>

#include "openpam_impl.h"

/*
 * OpenPAM extension
 *
 * Display an information message
 */

int
pam_info(const pam_handle_t *pamh,
	const char *fmt,
	...)
{
	va_list ap;
	char *rsp;
	int r;

	va_start(ap, fmt);
	r = pam_vprompt(pamh, PAM_TEXT_INFO, &rsp, fmt, ap);
	va_end(ap);
	FREE(rsp); /* ignore response */
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
 * The =pam_info function displays an informational message through the
 * intermediary of the given PAM context's conversation function.
 *
 * >pam_error
 * >pam_prompt
 * >pam_vinfo
 */