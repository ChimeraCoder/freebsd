
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

#ifdef HAVE_WINSOCK

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
inet_pton(int af, const char *csrc, void *dst)
{
    char * src;

    if (csrc == NULL || (src = strdup(csrc)) == NULL) {
	_set_errno( ENOMEM );
	return 0;
    }

    switch (af) {
    case AF_INET:
	{
	    struct sockaddr_in  si4;
	    INT r;
	    INT s = sizeof(si4);

	    si4.sin_family = AF_INET;
	    r = WSAStringToAddress(src, AF_INET, NULL, (LPSOCKADDR) &si4, &s);
	    free(src);
	    src = NULL;

	    if (r == 0) {
		memcpy(dst, &si4.sin_addr, sizeof(si4.sin_addr));
		return 1;
	    }
	}
	break;

    case AF_INET6:
	{
	    struct sockaddr_in6 si6;
	    INT r;
	    INT s = sizeof(si6);

	    si6.sin6_family = AF_INET6;
	    r = WSAStringToAddress(src, AF_INET6, NULL, (LPSOCKADDR) &si6, &s);
	    free(src);
	    src = NULL;

	    if (r == 0) {
		memcpy(dst, &si6.sin6_addr, sizeof(si6.sin6_addr));
		return 1;
	    }
	}
	break;

    default:
	_set_errno( EAFNOSUPPORT );
	return -1;
    }

    /* the call failed */
    {
	int le = WSAGetLastError();

	if (le == WSAEINVAL)
	    return 0;

	_set_errno(le);
	return -1;
    }
}

#else  /* !HAVE_WINSOCK */

ROKEN_LIB_FUNCTION int ROKEN_LIB_CALL
inet_pton(int af, const char *src, void *dst)
{
    if (af != AF_INET) {
	errno = EAFNOSUPPORT;
	return -1;
    }
    return inet_aton (src, dst);
}

#endif