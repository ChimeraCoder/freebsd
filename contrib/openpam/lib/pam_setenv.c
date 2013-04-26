
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
#include <stdio.h>
#include <string.h>

#include <security/pam_appl.h>

#include "openpam_impl.h"

/*
 * OpenPAM extension
 *
 * Set the value of an environment variable
 * Mirrors setenv(3)
 */

int
pam_setenv(pam_handle_t *pamh,
	const char *name,
	const char *value,
	int overwrite)
{
	char *env;
	int r;

	ENTER();
	if (pamh == NULL)
		RETURNC(PAM_SYSTEM_ERR);

	/* sanity checks */
	if (name == NULL || value == NULL || strchr(name, '=') != NULL)
		RETURNC(PAM_SYSTEM_ERR);

	/* is it already there? */
	if (!overwrite && openpam_findenv(pamh, name, strlen(name)) >= 0)
		RETURNC(PAM_SUCCESS);

	/* set it... */
	if (asprintf(&env, "%s=%s", name, value) < 0)
		RETURNC(PAM_BUF_ERR);
	r = pam_putenv(pamh, env);
	FREE(env);
	RETURNC(r);
}

/*
 * Error codes:
 *
 *	=pam_putenv
 *	PAM_SYSTEM_ERR
 *	PAM_BUF_ERR
 */

/**
 * The =pam_setenv function sets an environment variable.
 * Its semantics are similar to those of =setenv, but it modifies the PAM
 * context's environment list instead of the application's.
 *
 * >pam_getenv
 * >pam_getenvlist
 * >pam_putenv
 */