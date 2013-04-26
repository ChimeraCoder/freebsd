
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
#include <string.h>

#include "ypclnt.h"

ypclnt_t *
ypclnt_new(const char *domain, const char *map, const char *server)
{
	ypclnt_t *ypclnt;

	if ((ypclnt = calloc(1, sizeof *ypclnt)) == NULL)
		return (NULL);
	if (domain != NULL && (ypclnt->domain = strdup(domain)) == NULL)
		goto fail;
	if (map != NULL && (ypclnt->map = strdup(map)) == NULL)
		goto fail;
	if (server != NULL && (ypclnt->server = strdup(server)) == NULL)
		goto fail;
	return (ypclnt);
 fail:
	free(ypclnt->domain);
	free(ypclnt->map);
	free(ypclnt->server);
	free(ypclnt);
	return (NULL);
}