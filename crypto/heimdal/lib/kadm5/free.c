
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

void
kadm5_free_key_data(void *server_handle,
		    int16_t *n_key_data,
		    krb5_key_data *key_data)
{
    int i;
    for(i = 0; i < *n_key_data; i++){
	if(key_data[i].key_data_contents[0]){
	    memset(key_data[i].key_data_contents[0],
		   0,
		   key_data[i].key_data_length[0]);
	    free(key_data[i].key_data_contents[0]);
	}
	if(key_data[i].key_data_contents[1])
	    free(key_data[i].key_data_contents[1]);
    }
    *n_key_data = 0;
}


void
kadm5_free_principal_ent(void *server_handle,
			 kadm5_principal_ent_t princ)
{
    kadm5_server_context *context = server_handle;
    if(princ->principal)
	krb5_free_principal(context->context, princ->principal);
    if(princ->mod_name)
	krb5_free_principal(context->context, princ->mod_name);
    kadm5_free_key_data(server_handle, &princ->n_key_data, princ->key_data);
    while(princ->n_tl_data && princ->tl_data) {
	krb5_tl_data *tp;
	tp = princ->tl_data;
	princ->tl_data = tp->tl_data_next;
	princ->n_tl_data--;
	memset(tp->tl_data_contents, 0, tp->tl_data_length);
	free(tp->tl_data_contents);
	free(tp);
    }
    if (princ->key_data != NULL)
	free (princ->key_data);
}

void
kadm5_free_name_list(void *server_handle,
		     char **names,
		     int *count)
{
    int i;
    for(i = 0; i < *count; i++)
	free(names[i]);
    free(names);
    *count = 0;
}