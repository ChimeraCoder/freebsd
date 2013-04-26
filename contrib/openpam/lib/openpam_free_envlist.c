
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
 * OpenPAM extension
 *
 * Free an environment list
 */

void
openpam_free_envlist(char **envlist)
{
	char **env;

	ENTER();
	if (envlist == NULL)
		RETURNV();
	for (env = envlist; *env != NULL; ++env)
		FREE(*env);
	FREE(envlist);
	RETURNV();
}

/*
 * Error codes:
 */

/**
 * The =openpam_free_envlist function is a convenience function which
 * frees all the environment variables in an environment list, and the
 * list itself.
 * It is suitable for freeing the return value from =pam_getenvlist.
 *
 * AUTHOR DES
 */