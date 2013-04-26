
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

#include <string.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * OpenPAM extension
 *
 * Returns the value of a module option
 */

const char *
openpam_get_option(pam_handle_t *pamh,
	const char *option)
{
	pam_chain_t *cur;
	size_t len;
	int i;

	ENTERS(option);
	if (pamh == NULL || pamh->current == NULL || option == NULL)
		RETURNS(NULL);
	cur = pamh->current;
	len = strlen(option);
	for (i = 0; i < cur->optc; ++i) {
		if (strncmp(cur->optv[i], option, len) == 0) {
			if (cur->optv[i][len] == '\0')
				RETURNS(&cur->optv[i][len]);
			else if (cur->optv[i][len] == '=')
				RETURNS(&cur->optv[i][len + 1]);
		}
	}
	RETURNS(NULL);
}

/**
 * The =openpam_get_option function returns the value of the specified
 * option in the context of the currently executing service module, or
 * =NULL if the option is not set or no module is currently executing.
 *
 * >openpam_set_option
 */