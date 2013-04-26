
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

#include "includes.h"

#include "common.h"
#include "dh_groups.h"
#include "dh_group5.h"


void * dh5_init(struct wpabuf **priv, struct wpabuf **publ)
{
	*publ = dh_init(dh_groups_get(5), priv);
	if (*publ == 0)
		return NULL;
	return (void *) 1;
}


struct wpabuf * dh5_derive_shared(void *ctx, const struct wpabuf *peer_public,
				  const struct wpabuf *own_private)
{
	return dh_derive_shared(peer_public, own_private, dh_groups_get(5));
}


void dh5_free(void *ctx)
{
}