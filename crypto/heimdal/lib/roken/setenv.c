
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

#include "roken.h"

#include <stdlib.h>
#include <string.h>

/*
 * This is the easy way out, use putenv to implement setenv. We might
 * leak some memory but that is ok since we are usally about to exec
 * anyway.
 */

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
setenv(const char *var, const char *val, int rewrite)
{
#ifndef _WIN32
    char *t = NULL;

    if (!rewrite && getenv(var) != 0)
	return 0;

    if (asprintf (&t, "%s=%s", var, val) < 0 || t == NULL)
	return -1;

    if (putenv(t) == 0)
	return 0;
    else
	return -1;
#else  /* Win32 */
    char dummy[8];

    if (!rewrite && GetEnvironmentVariable(var, dummy, sizeof(dummy)/sizeof(char)) != 0)
	return 0;

    if (SetEnvironmentVariable(var, val) == 0)
	return -1;
    else
	return 0;
#endif
}