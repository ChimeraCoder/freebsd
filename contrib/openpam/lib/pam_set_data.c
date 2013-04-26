
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

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * XSSO 4.2.1
 * XSSO 6 page 59
 *
 * Set module information
 */

int
pam_set_data(pam_handle_t *pamh,
	const char *module_data_name,
	void *data,
	void (*cleanup)(pam_handle_t *pamh,
		void *data,
		int pam_end_status))
{
	pam_data_t *dp;

	ENTERS(module_data_name);
	if (pamh == NULL)
		RETURNC(PAM_SYSTEM_ERR);
	for (dp = pamh->module_data; dp != NULL; dp = dp->next) {
		if (strcmp(dp->name, module_data_name) == 0) {
			if (dp->cleanup)
				(dp->cleanup)(pamh, dp->data, PAM_SUCCESS);
			dp->data = data;
			dp->cleanup = cleanup;
			RETURNC(PAM_SUCCESS);
		}
	}
	if ((dp = malloc(sizeof *dp)) == NULL)
		RETURNC(PAM_BUF_ERR);
	if ((dp->name = strdup(module_data_name)) == NULL) {
		FREE(dp);
		RETURNC(PAM_BUF_ERR);
	}
	dp->data = data;
	dp->cleanup = cleanup;
	dp->next = pamh->module_data;
	pamh->module_data = dp;
	RETURNC(PAM_SUCCESS);
}

/*
 * Error codes:
 *
 *	PAM_SYSTEM_ERR
 *	PAM_BUF_ERR
 */

/**
 * The =pam_set_data function associates a pointer to an opaque object
 * with an arbitrary string specified by the =module_data_name argument,
 * in the PAM context specified by the =pamh argument.
 *
 * If not =NULL, the =cleanup argument should point to a function
 * responsible for releasing the resources associated with the object.
 *
 * This function and its counterpart =pam_get_data are useful for managing
 * data that are meaningful only to a particular service module.
 */