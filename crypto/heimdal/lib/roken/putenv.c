
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

#include <config.h>

#include <stdlib.h>

#if !HAVE_DECL_ENVIRON
extern char **environ;
#endif

/*
 * putenv --
 *	String points to a string of the form name=value.
 *
 *      Makes the value of the environment variable name equal to
 *      value by altering an existing variable or creating a new one.
 */

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
putenv(const char *string)
{
    int i;
    const char *eq = (const char *)strchr(string, '=');
    int len;

    if (eq == NULL)
	return 1;
    len = eq - string;

    if(environ == NULL) {
	environ = malloc(sizeof(char*));
	if(environ == NULL)
	    return 1;
	environ[0] = NULL;
    }

    for(i = 0; environ[i] != NULL; i++)
	if(strncmp(string, environ[i], len) == 0) {
	    environ[i] = string;
	    return 0;
	}
    environ = realloc(environ, sizeof(char*) * (i + 2));
    if(environ == NULL)
	return 1;
    environ[i]   = string;
    environ[i+1] = NULL;
    return 0;
}