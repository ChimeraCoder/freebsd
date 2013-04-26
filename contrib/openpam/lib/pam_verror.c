
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
#include <stdlib.h>

#include <security/pam_appl.h>
#include <security/openpam.h>

#include "openpam_impl.h"

/*
 * OpenPAM extension
 *
 * Display an error message
 */

int
pam_verror(const pam_handle_t *pamh,
	const char *fmt,
	va_list ap)
{
	char *rsp;
	int r;

	r = pam_vprompt(pamh, PAM_ERROR_MSG, &rsp, fmt, ap);
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
 * The =pam_verror function passes its arguments to =pam_vprompt with a
 * style argument of =PAM_ERROR_MSG, and discards the response.
 *
 * >pam_error
 * >pam_vinfo
 */