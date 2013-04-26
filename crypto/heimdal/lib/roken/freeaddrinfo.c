
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
 * free the list of `struct addrinfo' starting at `ai'
 */

ROKEN_LIB_FUNCTION void ROKEN_LIB_CALL
freeaddrinfo(struct addrinfo *ai)
{
    struct addrinfo *tofree;

    while(ai != NULL) {
	free (ai->ai_canonname);
	free (ai->ai_addr);
	tofree = ai;
	ai = ai->ai_next;
	free (tofree);
    }
}