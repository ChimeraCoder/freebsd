
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

#ifndef HAVE_H_ERRNO
static int h_errno = NO_RECOVERY;
#endif

/*
 * lookup `name' (address family `af') in DNS and return a pointer
 * to a malloced struct hostent or NULL.
 */

ROKEN_LIB_FUNCTION struct hostent * ROKEN_LIB_CALL
getipnodebyname (const char *name, int af, int flags, int *error_num)
{
    struct hostent *tmp;

#ifdef HAVE_GETHOSTBYNAME2
    tmp = gethostbyname2 (name, af);
#else
    if (af != AF_INET) {
	*error_num = NO_ADDRESS;
	return NULL;
    }
    tmp = gethostbyname (name);
#endif
    if (tmp == NULL) {
	switch (h_errno) {
	case HOST_NOT_FOUND :
	case TRY_AGAIN :
	case NO_RECOVERY :
	    *error_num = h_errno;
	    break;
	case NO_DATA :
	    *error_num = NO_ADDRESS;
	    break;
	default :
	    *error_num = NO_RECOVERY;
	    break;
	}
	return NULL;
    }
    tmp = copyhostent (tmp);
    if (tmp == NULL) {
	*error_num = TRY_AGAIN;
	return NULL;
    }
    return tmp;
}