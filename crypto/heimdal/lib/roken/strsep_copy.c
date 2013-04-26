
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

#include <string.h>

#include "roken.h"

#ifndef HAVE_STRSEP_COPY

/* strsep, but with const stringp, so return string in buf */

ROKEN_LIB_FUNCTION ssize_t ROKEN_LIB_CALL
strsep_copy(const char **stringp, const char *delim, char *buf, size_t len)
{
    const char *save = *stringp;
    size_t l;
    if(save == NULL)
	return -1;
    *stringp = *stringp + strcspn(*stringp, delim);
    l = min(len, (size_t)(*stringp - save));
    if(len > 0) {
	memcpy(buf, save, l);
	buf[l] = '\0';
    }

    l = *stringp - save;
    if(**stringp == '\0')
	*stringp = NULL;
    else
	(*stringp)++;
    return l;
}

#endif