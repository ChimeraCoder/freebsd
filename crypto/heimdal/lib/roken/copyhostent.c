
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

/*
 * return a malloced copy of `h'
 */

ROKEN_LIB_FUNCTION struct hostent * ROKEN_LIB_CALL
copyhostent (const struct hostent *h)
{
    struct hostent *res;
    char **p;
    int i, n;

    res = malloc (sizeof (*res));
    if (res == NULL)
	return NULL;
    res->h_name      = NULL;
    res->h_aliases   = NULL;
    res->h_addrtype  = h->h_addrtype;
    res->h_length    = h->h_length;
    res->h_addr_list = NULL;
    res->h_name = strdup (h->h_name);
    if (res->h_name == NULL) {
	freehostent (res);
	return NULL;
    }
    for (n = 0, p = h->h_aliases; *p != NULL; ++p)
	++n;
    res->h_aliases = malloc ((n + 1) * sizeof(*res->h_aliases));
    if (res->h_aliases == NULL) {
	freehostent (res);
	return NULL;
    }
    for (i = 0; i < n + 1; ++i)
	res->h_aliases[i] = NULL;
    for (i = 0; i < n; ++i) {
	res->h_aliases[i] = strdup (h->h_aliases[i]);
	if (res->h_aliases[i] == NULL) {
	    freehostent (res);
	    return NULL;
	}
    }

    for (n = 0, p = h->h_addr_list; *p != NULL; ++p)
	++n;
    res->h_addr_list = malloc ((n + 1) * sizeof(*res->h_addr_list));
    if (res->h_addr_list == NULL) {
	freehostent (res);
	return NULL;
    }
    for (i = 0; i < n + 1; ++i) {
	res->h_addr_list[i] = NULL;
    }
    for (i = 0; i < n; ++i) {
	res->h_addr_list[i] = malloc (h->h_length);
	if (res->h_addr_list[i] == NULL) {
	    freehostent (res);
	    return NULL;
	}
	memcpy (res->h_addr_list[i], h->h_addr_list[i], h->h_length);
    }
    return res;
}