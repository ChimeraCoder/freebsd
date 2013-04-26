
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

int _krb5_AES_string_to_default_iterator = 4096;

static krb5_error_code
AES_string_to_key(krb5_context context,
		  krb5_enctype enctype,
		  krb5_data password,
		  krb5_salt salt,
		  krb5_data opaque,
		  krb5_keyblock *key)
{
    krb5_error_code ret;
    uint32_t iter;
    struct _krb5_encryption_type *et;
    struct _krb5_key_data kd;

    if (opaque.length == 0)
	iter = _krb5_AES_string_to_default_iterator;
    else if (opaque.length == 4) {
	unsigned long v;
	_krb5_get_int(opaque.data, &v, 4);
	iter = ((uint32_t)v);
    } else
	return KRB5_PROG_KEYTYPE_NOSUPP; /* XXX */

    et = _krb5_find_enctype(enctype);
    if (et == NULL)
	return KRB5_PROG_KEYTYPE_NOSUPP;

    kd.schedule = NULL;
    ALLOC(kd.key, 1);
    if(kd.key == NULL) {
	krb5_set_error_message (context, ENOMEM, N_("malloc: out of memory", ""));
	return ENOMEM;
    }
    kd.key->keytype = enctype;
    ret = krb5_data_alloc(&kd.key->keyvalue, et->keytype->size);
    if (ret) {
	krb5_set_error_message (context, ret, N_("malloc: out of memory", ""));
	return ret;
    }

    ret = PKCS5_PBKDF2_HMAC_SHA1(password.data, password.length,
				 salt.saltvalue.data, salt.saltvalue.length,
				 iter,
				 et->keytype->size, kd.key->keyvalue.data);
    if (ret != 1) {
	_krb5_free_key_data(context, &kd, et);
	krb5_set_error_message(context, KRB5_PROG_KEYTYPE_NOSUPP,
			       "Error calculating s2k");
	return KRB5_PROG_KEYTYPE_NOSUPP;
    }

    ret = _krb5_derive_key(context, et, &kd, "kerberos", strlen("kerberos"));
    if (ret == 0)
	ret = krb5_copy_keyblock_contents(context, kd.key, key);
    _krb5_free_key_data(context, &kd, et);

    return ret;
}

struct salt_type _krb5_AES_salt[] = {
    {
	KRB5_PW_SALT,
	"pw-salt",
	AES_string_to_key
    },
    { 0 }
};