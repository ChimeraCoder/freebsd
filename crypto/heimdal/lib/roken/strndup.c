
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
#include <string.h>

#include "roken.h"

#ifndef HAVE_STRNDUP
ROKEN_LIB_FUNCTION char * ROKEN_LIB_CALL
strndup(const char *old, size_t sz)
{
    size_t len = strnlen (old, sz);
    char *t    = malloc(len + 1);

    if (t != NULL) {
	memcpy (t, old, len);
	t[len] = '\0';
    }
    return t;
}
#endif /* HAVE_STRNDUP */