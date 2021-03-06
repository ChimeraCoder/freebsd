
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

#ifdef DES3_OLD_ENCTYPE
static krb5_error_code
DES3_string_to_key(krb5_context context,
		   krb5_enctype enctype,
		   krb5_data password,
		   krb5_salt salt,
		   krb5_data opaque,
		   krb5_keyblock *key)
{
    char *str;
    size_t len;
    unsigned char tmp[24];
    DES_cblock keys[3];
    krb5_error_code ret;

    len = password.length + salt.saltvalue.length;
    str = malloc(len);
    if(len != 0 && str == NULL) {
	krb5_set_error_message(context, ENOMEM, N_("malloc: out of memory", ""));
	return ENOMEM;
    }
    memcpy(str, password.data, password.length);
    memcpy(str + password.length, salt.saltvalue.data, salt.saltvalue.length);
    {
	DES_cblock ivec;
	DES_key_schedule s[3];
	int i;

	ret = _krb5_n_fold(str, len, tmp, 24);
	if (ret) {
	    memset(str, 0, len);
	    free(str);
	    krb5_set_error_message(context, ret, N_("malloc: out of memory", ""));
	    return ret;
	}

	for(i = 0; i < 3; i++){
	    memcpy(keys + i, tmp + i * 8, sizeof(keys[i]));
	    DES_set_odd_parity(keys + i);
	    if(DES_is_weak_key(keys + i))
		_krb5_xor(keys + i, (const unsigned char*)"\0\0\0\0\0\0\0\xf0");
	    DES_set_key_unchecked(keys + i, &s[i]);
	}
	memset(&ivec, 0, sizeof(ivec));
	DES_ede3_cbc_encrypt(tmp,
			     tmp, sizeof(tmp),
			     &s[0], &s[1], &s[2], &ivec, DES_ENCRYPT);
	memset(s, 0, sizeof(s));
	memset(&ivec, 0, sizeof(ivec));
	for(i = 0; i < 3; i++){
	    memcpy(keys + i, tmp + i * 8, sizeof(keys[i]));
	    DES_set_odd_parity(keys + i);
	    if(DES_is_weak_key(keys + i))
		_krb5_xor(keys + i, (const unsigned char*)"\0\0\0\0\0\0\0\xf0");
	}
	memset(tmp, 0, sizeof(tmp));
    }
    key->keytype = enctype;
    krb5_data_copy(&key->keyvalue, keys, sizeof(keys));
    memset(keys, 0, sizeof(keys));
    memset(str, 0, len);
    free(str);
    return 0;
}
#endif

static krb5_error_code
DES3_string_to_key_derived(krb5_context context,
			   krb5_enctype enctype,
			   krb5_data password,
			   krb5_salt salt,
			   krb5_data opaque,
			   krb5_keyblock *key)
{
    krb5_error_code ret;
    size_t len = password.length + salt.saltvalue.length;
    char *s;

    s = malloc(len);
    if(len != 0 && s == NULL) {
	krb5_set_error_message(context, ENOMEM, N_("malloc: out of memory", ""));
	return ENOMEM;
    }
    memcpy(s, password.data, password.length);
    memcpy(s + password.length, salt.saltvalue.data, salt.saltvalue.length);
    ret = krb5_string_to_key_derived(context,
				     s,
				     len,
				     enctype,
				     key);
    memset(s, 0, len);
    free(s);
    return ret;
}


#ifdef DES3_OLD_ENCTYPE
struct salt_type _krb5_des3_salt[] = {
    {
	KRB5_PW_SALT,
	"pw-salt",
	DES3_string_to_key
    },
    { 0 }
};
#endif

struct salt_type _krb5_des3_salt_derived[] = {
    {
	KRB5_PW_SALT,
	"pw-salt",
	DES3_string_to_key_derived
    },
    { 0 }
};