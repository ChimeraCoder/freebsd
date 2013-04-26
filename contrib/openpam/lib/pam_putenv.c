
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
 * XSSO 6 page 56
 *
 * Set the value of an environment variable
 */

int
pam_putenv(pam_handle_t *pamh,
	const char *namevalue)
{
	char **env, *p;
	int i;

	ENTER();
	if (pamh == NULL)
		RETURNC(PAM_SYSTEM_ERR);

	/* sanity checks */
	if (namevalue == NULL || (p = strchr(namevalue, '=')) == NULL)
		RETURNC(PAM_SYSTEM_ERR);

	/* see if the variable is already in the environment */
	if ((i = openpam_findenv(pamh, namevalue, p - namevalue)) >= 0) {
		if ((p = strdup(namevalue)) == NULL)
			RETURNC(PAM_BUF_ERR);
		FREE(pamh->env[i]);
		pamh->env[i] = p;
		RETURNC(PAM_SUCCESS);
	}

	/* grow the environment list if necessary */
	if (pamh->env_count == pamh->env_size) {
		env = realloc(pamh->env,
		    sizeof(char *) * (pamh->env_size * 2 + 1));
		if (env == NULL)
			RETURNC(PAM_BUF_ERR);
		pamh->env = env;
		pamh->env_size = pamh->env_size * 2 + 1;
	}

	/* add the variable at the end */
	if ((pamh->env[pamh->env_count] = strdup(namevalue)) == NULL)
		RETURNC(PAM_BUF_ERR);
	++pamh->env_count;
	RETURNC(PAM_SUCCESS);
}

/*
 * Error codes:
 *
 *	PAM_SYSTEM_ERR
 *	PAM_BUF_ERR
 */

/**
 * The =pam_putenv function sets an environment variable.
 * Its semantics are similar to those of =putenv, but it modifies the PAM
 * context's environment list instead of the application's.
 *
 * >pam_getenv
 * >pam_getenvlist
 * >pam_setenv
 */