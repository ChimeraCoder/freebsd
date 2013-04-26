
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
 * XSSO 6 page 45
 *
 * Returns a list of all the PAM environment variables
 */

char **
pam_getenvlist(pam_handle_t *pamh)
{
	char **envlist;
	int i;

	ENTER();
	if (pamh == NULL)
		RETURNP(NULL);
	envlist = malloc(sizeof(char *) * (pamh->env_count + 1));
	if (envlist == NULL) {
		openpam_log(PAM_LOG_ERROR, "%s",
			pam_strerror(pamh, PAM_BUF_ERR));
		RETURNP(NULL);
	}
	for (i = 0; i < pamh->env_count; ++i) {
		if ((envlist[i] = strdup(pamh->env[i])) == NULL) {
			while (i) {
				--i;
				FREE(envlist[i]);
			}
			FREE(envlist);
			openpam_log(PAM_LOG_ERROR, "%s",
				pam_strerror(pamh, PAM_BUF_ERR));
			RETURNP(NULL);
		}
	}
	envlist[i] = NULL;
	RETURNP(envlist);
}

/**
 * The =pam_getenvlist function returns a copy of the given PAM context's
 * environment list as a pointer to an array of strings.
 * The last element in the array is =NULL.
 * The pointer is suitable for assignment to {Va environ}.
 *
 * The array and the strings it lists are allocated using =malloc, and
 * should be released using =free after use:
 *
 *     char **envlist, **env;
 *
 *     envlist = environ;
 *     environ = pam_getenvlist(pamh);
 *     \/\* do something nifty \*\/
 *     for (env = environ; *env != NULL; env++)
 *         free(*env);
 *     free(environ);
 *     environ = envlist;
 *
 * >environ 7
 * >pam_getenv
 * >pam_putenv
 * >pam_setenv
 */