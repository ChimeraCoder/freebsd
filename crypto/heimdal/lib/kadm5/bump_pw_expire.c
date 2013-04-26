
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

#include "kadm5_locl.h"

RCSID("$Id$");

/*
 * extend password_expiration if it's defined
 */

kadm5_ret_t
_kadm5_bump_pw_expire(kadm5_server_context *context,
		      hdb_entry *ent)
{
    if (ent->pw_end != NULL) {
	time_t life;

	life = krb5_config_get_time_default(context->context,
					    NULL,
					    365 * 24 * 60 * 60,
					    "kadmin",
					    "password_lifetime",
					    NULL);

	*(ent->pw_end) = time(NULL) + life;
    }
    return 0;
}