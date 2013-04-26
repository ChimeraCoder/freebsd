
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * OpenPAM extension
 *
 * Sets the value of a module option
 */

int
openpam_set_option(pam_handle_t *pamh,
	const char *option,
	const char *value)
{
	pam_chain_t *cur;
	char *opt, **optv;
	size_t len;
	int i;

	ENTERS(option);
	if (pamh == NULL || pamh->current == NULL || option == NULL)
		RETURNC(PAM_SYSTEM_ERR);
	cur = pamh->current;
	for (len = 0; option[len] != '\0'; ++len)
		if (option[len] == '=')
			break;
	for (i = 0; i < cur->optc; ++i) {
		if (strncmp(cur->optv[i], option, len) == 0 &&
		    (cur->optv[i][len] == '\0' || cur->optv[i][len] == '='))
			break;
	}
	if (value == NULL) {
		/* remove */
		if (i == cur->optc)
			RETURNC(PAM_SUCCESS);
		for (free(cur->optv[i]); i < cur->optc; ++i)
			cur->optv[i] = cur->optv[i + 1];
		cur->optv[i] = NULL;
		RETURNC(PAM_SUCCESS);
	}
	if (asprintf(&opt, "%.*s=%s", (int)len, option, value) < 0)
		RETURNC(PAM_BUF_ERR);
	if (i == cur->optc) {
		/* add */
		optv = realloc(cur->optv, sizeof(char *) * (cur->optc + 2));
		if (optv == NULL) {
			FREE(opt);
			RETURNC(PAM_BUF_ERR);
		}
		optv[i] = opt;
		optv[i + 1] = NULL;
		cur->optv = optv;
		++cur->optc;
	} else {
		/* replace */
		FREE(cur->optv[i]);
		cur->optv[i] = opt;
	}
	RETURNC(PAM_SUCCESS);
}

/*
 * Error codes:
 *
 *	PAM_SYSTEM_ERR
 *	PAM_BUF_ERR
 */

/**
 * The =openpam_set_option function sets the specified option in the
 * context of the currently executing service module.
 *
 * >openpam_get_option
 */