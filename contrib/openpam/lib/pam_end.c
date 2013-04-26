
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

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * XSSO 4.2.1
 * XSSO 6 page 42
 *
 * Terminate the PAM transaction
 */

int
pam_end(pam_handle_t *pamh,
	int status)
{
	pam_data_t *dp;
	int i;

	ENTER();
	if (pamh == NULL)
		RETURNC(PAM_SYSTEM_ERR);

	/* clear module data */
	while ((dp = pamh->module_data) != NULL) {
		if (dp->cleanup)
			(dp->cleanup)(pamh, dp->data, status);
		pamh->module_data = dp->next;
		FREE(dp->name);
		FREE(dp);
	}

	/* clear environment */
	while (pamh->env_count) {
		--pamh->env_count;
		FREE(pamh->env[pamh->env_count]);
	}
	FREE(pamh->env);

	/* clear chains */
	openpam_clear_chains(pamh->chains);

	/* clear items */
	for (i = 0; i < PAM_NUM_ITEMS; ++i)
		pam_set_item(pamh, i, NULL);

	FREE(pamh);

	RETURNC(PAM_SUCCESS);
}

/*
 * Error codes:
 *
 *	PAM_SYSTEM_ERR
 */

/**
 * The =pam_end function terminates a PAM transaction and destroys the
 * corresponding PAM context, releasing all resources allocated to it.
 *
 * The =status argument should be set to the error code returned by the
 * last API call before the call to =pam_end.
 */