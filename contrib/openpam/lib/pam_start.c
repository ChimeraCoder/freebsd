
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"
#include "openpam_strlcpy.h"

#ifdef _SC_HOST_NAME_MAX
#define HOST_NAME_MAX sysconf(_SC_HOST_NAME_MAX)
#else
#define HOST_NAME_MAX 1024
#endif

/*
 * XSSO 4.2.1
 * XSSO 6 page 89
 *
 * Initiate a PAM transaction
 */

int
pam_start(const char *service,
	const char *user,
	const struct pam_conv *pam_conv,
	pam_handle_t **pamh)
{
	char hostname[HOST_NAME_MAX + 1];
	struct pam_handle *ph;
	int r;

	ENTER();
	if ((ph = calloc(1, sizeof *ph)) == NULL)
		RETURNC(PAM_BUF_ERR);
	if ((r = pam_set_item(ph, PAM_SERVICE, service)) != PAM_SUCCESS)
		goto fail;
	if (gethostname(hostname, sizeof hostname) != 0)
		strlcpy(hostname, "localhost", sizeof hostname);
	if ((r = pam_set_item(ph, PAM_HOST, hostname)) != PAM_SUCCESS)
		goto fail;
	if ((r = pam_set_item(ph, PAM_USER, user)) != PAM_SUCCESS)
		goto fail;
	if ((r = pam_set_item(ph, PAM_CONV, pam_conv)) != PAM_SUCCESS)
		goto fail;
	if ((r = openpam_configure(ph, service)) != PAM_SUCCESS)
		goto fail;
	*pamh = ph;
	openpam_log(PAM_LOG_DEBUG, "pam_start(\"%s\") succeeded", service);
	RETURNC(PAM_SUCCESS);
fail:
	pam_end(ph, r);
	RETURNC(r);
}

/*
 * Error codes:
 *
 *	=openpam_configure
 *	=pam_set_item
 *	!PAM_SYMBOL_ERR
 *	PAM_BUF_ERR
 */

/**
 * The =pam_start function creates and initializes a PAM context.
 *
 * The =service argument specifies the name of the policy to apply, and is
 * stored in the =PAM_SERVICE item in the created context.
 *
 * The =user argument specifies the name of the target user - the user the
 * created context will serve to authenticate.
 * It is stored in the =PAM_USER item in the created context.
 *
 * The =pam_conv argument points to a =struct pam_conv describing the
 * conversation function to use; see =pam_conv for details.
 *
 * >pam_get_item
 * >pam_set_item
 * >pam_end
 */