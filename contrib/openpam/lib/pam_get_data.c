
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

#include <string.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * XSSO 4.2.1
 * XSSO 6 page 43
 *
 * Get module information
 */

int
pam_get_data(const pam_handle_t *pamh,
	const char *module_data_name,
	const void **data)
{
	pam_data_t *dp;

	ENTERS(module_data_name);
	if (pamh == NULL)
		RETURNC(PAM_SYSTEM_ERR);
	for (dp = pamh->module_data; dp != NULL; dp = dp->next) {
		if (strcmp(dp->name, module_data_name) == 0) {
			*data = (void *)dp->data;
			RETURNC(PAM_SUCCESS);
		}
	}
	RETURNC(PAM_NO_MODULE_DATA);
}

/*
 * Error codes:
 *
 *	PAM_SYSTEM_ERR
 *	PAM_NO_MODULE_DATA
 */

/**
 * The =pam_get_data function looks up the opaque object associated with
 * the string specified by the =module_data_name argument, in the PAM
 * context specified by the =pamh argument.
 * A pointer to the object is stored in the location pointed to by the
 * =data argument.
 * If =pam_get_data fails, the =data argument is untouched.
 *
 * This function and its counterpart =pam_set_data are useful for managing
 * data that are meaningful only to a particular service module.
 */