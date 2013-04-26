
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

#include <stdlib.h>

#include "ypclnt.h"

void
ypclnt_free(ypclnt_t *ypclnt)
{
	if (ypclnt != NULL) {
		free(ypclnt->domain);
		free(ypclnt->map);
		free(ypclnt->server);
		free(ypclnt->error);
		free(ypclnt);
	}
}