
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

#include "kadmin_locl.h"
#include "kadmin-commands.h"

static int
do_del_entry(krb5_principal principal, void *data)
{
    return kadm5_delete_principal(kadm_handle, principal);
}

int
del_entry(void *opt, int argc, char **argv)
{
    int i;
    krb5_error_code ret = 0;

    for(i = 0; i < argc; i++) {
	ret = foreach_principal(argv[i], do_del_entry, "del", NULL);
	if (ret)
	    break;
    }
    return ret != 0;
}