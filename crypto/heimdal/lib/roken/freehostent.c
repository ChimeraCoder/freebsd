
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
 * free a malloced hostent
 */

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
freehostent (struct hostent *h)
{
    char **p;

    free (h->h_name);
    if (h->h_aliases != NULL) {
	for (p = h->h_aliases; *p != NULL; ++p)
	    free (*p);
	free (h->h_aliases);
    }
    if (h->h_addr_list != NULL) {
	for (p = h->h_addr_list; *p != NULL; ++p)
	    free (*p);
	free (h->h_addr_list);
    }
    free (h);
}