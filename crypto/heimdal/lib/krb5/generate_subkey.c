
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

/**
 * Generate subkey, from keyblock
 *
 * @param context kerberos context
 * @param key session key
 * @param etype encryption type of subkey, if ETYPE_NULL, use key's enctype
 * @param subkey returned new, free with krb5_free_keyblock().
 *
 * @return 0 on success or a Kerberos 5 error code
 *
* @ingroup krb5_crypto
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_generate_subkey_extended(krb5_context context,
			      const krb5_keyblock *key,
			      krb5_enctype etype,
			      krb5_keyblock **subkey)
{
    krb5_error_code ret;

    ALLOC(*subkey, 1);
    if (*subkey == NULL) {
	krb5_set_error_message(context, ENOMEM,N_("malloc: out of memory", ""));
	return ENOMEM;
    }

    if (etype == ETYPE_NULL)
	etype = key->keytype; /* use session key etype */

    /* XXX should we use the session key as input to the RF? */
    ret = krb5_generate_random_keyblock(context, etype, *subkey);
    if (ret != 0) {
	free(*subkey);
	*subkey = NULL;
    }

    return ret;
}