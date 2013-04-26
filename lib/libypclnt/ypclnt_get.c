
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

#include "ypclnt.h"

char *
ypclnt_get(ypclnt_t *ypc, const char *key)
{
	char *value;
	int len, r;

	r = yp_match(ypc->domain, ypc->map,
	    key, (int)strlen(key), &value, &len);
	if (r != 0) {
		ypclnt_error(ypc, __func__, "%s", yperr_string(r));
		return (NULL);
	}
	return (value);
}