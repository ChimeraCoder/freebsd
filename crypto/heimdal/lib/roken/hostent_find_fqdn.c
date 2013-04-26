
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
 * Try to find a fqdn (with `.') in he if possible, else return h_name
 */

ROKEN_LIB_FUNCTION const char * ROKEN_LIB_CALL
hostent_find_fqdn (const struct hostent *he)
{
    const char *ret = he->h_name;
    const char **h;

    if (strchr (ret, '.') == NULL)
	for (h = (const char **)he->h_aliases; *h != NULL; ++h) {
	    if (strchr (*h, '.') != NULL) {
		ret = *h;
		break;
	    }
	}
    return ret;
}