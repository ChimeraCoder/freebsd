
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

#include "krb5_locl.h"

static krb5_error_code
ARCFOUR_string_to_key(krb5_context context,
		      krb5_enctype enctype,
		      krb5_data password,
		      krb5_salt salt,
		      krb5_data opaque,
		      krb5_keyblock *key)
{
    krb5_error_code ret;
    uint16_t *s = NULL;
    size_t len = 0, i;
    EVP_MD_CTX *m;

    m = EVP_MD_CTX_create();
    if (m == NULL) {
	ret = ENOMEM;
	krb5_set_error_message(context, ret, N_("malloc: out of memory", ""));
	goto out;
    }

    EVP_DigestInit_ex(m, EVP_md4(), NULL);

    ret = wind_utf8ucs2_length(password.data, &len);
    if (ret) {
	krb5_set_error_message (context, ret,
				N_("Password not an UCS2 string", ""));
	goto out;
    }

    s = malloc (len * sizeof(s[0]));
    if (len != 0 && s == NULL) {
	krb5_set_error_message (context, ENOMEM,
				N_("malloc: out of memory", ""));
	ret = ENOMEM;
	goto out;
    }

    ret = wind_utf8ucs2(password.data, s, &len);
    if (ret) {
	krb5_set_error_message (context, ret,
				N_("Password not an UCS2 string", ""));
	goto out;
    }

    /* LE encoding */
    for (i = 0; i < len; i++) {
	unsigned char p;
	p = (s[i] & 0xff);
	EVP_DigestUpdate (m, &p, 1);
	p = (s[i] >> 8) & 0xff;
	EVP_DigestUpdate (m, &p, 1);
    }

    key->keytype = enctype;
    ret = krb5_data_alloc (&key->keyvalue, 16);
    if (ret) {
	krb5_set_error_message (context, ENOMEM, N_("malloc: out of memory", ""));
	goto out;
    }
    EVP_DigestFinal_ex (m, key->keyvalue.data, NULL);

 out:
    EVP_MD_CTX_destroy(m);
    if (s)
	memset (s, 0, len);
    free (s);
    return ret;
}

struct salt_type _krb5_arcfour_salt[] = {
    {
	KRB5_PW_SALT,
	"pw-salt",
	ARCFOUR_string_to_key
    },
    { 0 }
};