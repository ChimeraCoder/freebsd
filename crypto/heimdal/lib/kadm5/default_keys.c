
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
#include <err.h>

RCSID("$Id$");

static void
print_keys(krb5_context context, Key *keys, size_t nkeys)
{
    krb5_error_code ret;
    char *str;
    int i;

    printf("keys:\n");

    for (i = 0; i < nkeys; i++) {

	ret = krb5_enctype_to_string(context, keys[i].key.keytype, &str);
	if (ret)
	    krb5_err(context, ret, 1, "krb5_enctype_to_string: %d\n",
		     (int)keys[i].key.keytype);

	printf("\tenctype %s", str);
	free(str);

	if (keys[i].salt) {
	    printf(" salt: ");

	    switch (keys[i].salt->type) {
	    case KRB5_PW_SALT:
		printf("pw-salt:");
		break;
	    case KRB5_AFS3_SALT:
		printf("afs3-salt:");
		break;
	    default:
		printf("unknown salt: %d", keys[i].salt->type);
		break;
	    }
	    if (keys[i].salt->salt.length)
		printf("%.*s", (int)keys[i].salt->salt.length,
		       (char *)keys[i].salt->salt.data);
	}
	printf("\n");
    }
    printf("end keys:\n");
}

static void
parse_file(krb5_context context, krb5_principal principal, int no_salt)
{
    krb5_error_code ret;
    size_t nkeys;
    Key *keys;

    ret = hdb_generate_key_set(context, principal, &keys, &nkeys, no_salt);
    if (ret)
	krb5_err(context, 1, ret, "hdb_generate_key_set");

    print_keys(context, keys, nkeys);

    hdb_free_keys(context, nkeys, keys);
}

int
main(int argc, char **argv)
{
    krb5_error_code ret;
    krb5_context context;
    krb5_principal principal;

    ret = krb5_init_context(&context);
    if (ret)
	errx(1, "krb5_init_context");

    ret = krb5_parse_name(context, "lha@SU.SE", &principal);
    if (ret)
	krb5_err(context, ret, 1, "krb5_parse_name");

    parse_file(context, principal, 0);
    parse_file(context, principal, 1);

    krb5_free_principal(context, principal);

    krb5_free_context(context);

    return 0;
}