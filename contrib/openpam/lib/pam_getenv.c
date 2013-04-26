
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
 * XSSO 6 page 44
 *
 * Retrieve the value of a PAM environment variable
 */

const char *
pam_getenv(pam_handle_t *pamh,
	const char *name)
{
	char *str;
	int i;

	ENTERS(name);
	if (pamh == NULL)
		RETURNS(NULL);
	if (name == NULL || strchr(name, '=') != NULL)
		RETURNS(NULL);
	if ((i = openpam_findenv(pamh, name, strlen(name))) < 0)
		RETURNS(NULL);
	for (str = pamh->env[i]; *str != '\0'; ++str) {
		if (*str == '=') {
			++str;
			break;
		}
	}
	RETURNS(str);
}

/**
 * The =pam_getenv function returns the value of an environment variable.
 * Its semantics are similar to those of =getenv, but it accesses the PAM
 * context's environment list instead of the application's.
 *
 * >pam_getenvlist
 * >pam_putenv
 * >pam_setenv
 */