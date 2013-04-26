
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

kadm5_ret_t
_kadm5_set_modifier(kadm5_server_context *context,
		    hdb_entry *ent)
{
    kadm5_ret_t ret;
    if(ent->modified_by == NULL){
	ent->modified_by = malloc(sizeof(*ent->modified_by));
	if(ent->modified_by == NULL)
	    return ENOMEM;
    } else
	free_Event(ent->modified_by);
    ent->modified_by->time = time(NULL);
    ret = krb5_copy_principal(context->context, context->caller,
			      &ent->modified_by->principal);
    return ret;
}