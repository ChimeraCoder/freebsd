
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
#include <kadm5/private.h>

extern int local_flag;

int
dump(struct dump_options *opt, int argc, char **argv)
{
    krb5_error_code ret;
    FILE *f;
    HDB *db = NULL;

    if(!local_flag) {
	krb5_warnx(context, "dump is only available in local (-l) mode");
	return 0;
    }

    db = _kadm5_s_get_db(kadm_handle);

    if(argc == 0)
	f = stdout;
    else
	f = fopen(argv[0], "w");

    if(f == NULL) {
	krb5_warn(context, errno, "open: %s", argv[0]);
	goto out;
    }
    ret = db->hdb_open(context, db, O_RDONLY, 0600);
    if(ret) {
	krb5_warn(context, ret, "hdb_open");
	goto out;
    }

    hdb_foreach(context, db, opt->decrypt_flag ? HDB_F_DECRYPT : 0,
		hdb_print_entry, f);

    db->hdb_close(context, db);
out:
    if(f && f != stdout)
	fclose(f);
    return 0;
}