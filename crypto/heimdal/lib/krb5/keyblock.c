
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
 * Zero out a keyblock
 *
 * @param keyblock keyblock to zero out
 *
 * @ingroup krb5_crypto
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_keyblock_zero(krb5_keyblock *keyblock)
{
    keyblock->keytype = 0;
    krb5_data_zero(&keyblock->keyvalue);
}

/**
 * Free a keyblock's content, also zero out the content of the keyblock.
 *
 * @param context a Kerberos 5 context
 * @param keyblock keyblock content to free, NULL is valid argument
 *
 * @ingroup krb5_crypto
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_free_keyblock_contents(krb5_context context,
			    krb5_keyblock *keyblock)
{
    if(keyblock) {
	if (keyblock->keyvalue.data != NULL)
	    memset(keyblock->keyvalue.data, 0, keyblock->keyvalue.length);
	krb5_data_free (&keyblock->keyvalue);
	keyblock->keytype = ENCTYPE_NULL;
    }
}

/**
 * Free a keyblock, also zero out the content of the keyblock, uses
 * krb5_free_keyblock_contents() to free the content.
 *
 * @param context a Kerberos 5 context
 * @param keyblock keyblock to free, NULL is valid argument
 *
 * @ingroup krb5_crypto
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_free_keyblock(krb5_context context,
		   krb5_keyblock *keyblock)
{
    if(keyblock){
	krb5_free_keyblock_contents(context, keyblock);
	free(keyblock);
    }
}

/**
 * Copy a keyblock, free the output keyblock with
 * krb5_free_keyblock_contents().
 *
 * @param context a Kerberos 5 context
 * @param inblock the key to copy
 * @param to the output key.
 *
 * @return 0 on success or a Kerberos 5 error code
 *
 * @ingroup krb5_crypto
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_copy_keyblock_contents (krb5_context context,
			     const krb5_keyblock *inblock,
			     krb5_keyblock *to)
{
    return copy_EncryptionKey(inblock, to);
}

/**
 * Copy a keyblock, free the output keyblock with
 * krb5_free_keyblock().
 *
 * @param context a Kerberos 5 context
 * @param inblock the key to copy
 * @param to the output key.
 *
 * @return 0 on success or a Kerberos 5 error code
 *
 * @ingroup krb5_crypto
 */


KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_copy_keyblock (krb5_context context,
		    const krb5_keyblock *inblock,
		    krb5_keyblock **to)
{
    krb5_error_code ret;
    krb5_keyblock *k;

    *to = NULL;

    k = calloc (1, sizeof(*k));
    if (k == NULL) {
	krb5_set_error_message(context, ENOMEM, "malloc: out of memory");
	return ENOMEM;
    }

    ret = krb5_copy_keyblock_contents (context, inblock, k);
    if (ret) {
      free(k);
      return ret;
    }
    *to = k;
    return 0;
}

/**
 * Get encryption type of a keyblock.
 *
 * @ingroup krb5_crypto
 */

KRB5_LIB_FUNCTION krb5_enctype KRB5_LIB_CALL
krb5_keyblock_get_enctype(const krb5_keyblock *block)
{
    return block->keytype;
}

/**
 * Fill in `key' with key data of type `enctype' from `data' of length
 * `size'. Key should be freed using krb5_free_keyblock_contents().
 *
 * @return 0 on success or a Kerberos 5 error code
 *
 * @ingroup krb5_crypto
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_keyblock_init(krb5_context context,
		   krb5_enctype type,
		   const void *data,
		   size_t size,
		   krb5_keyblock *key)
{
    krb5_error_code ret;
    size_t len;

    memset(key, 0, sizeof(*key));

    ret = krb5_enctype_keysize(context, type, &len);
    if (ret)
	return ret;

    if (len != size) {
	krb5_set_error_message(context, KRB5_PROG_ETYPE_NOSUPP,
			       "Encryption key %d is %lu bytes "
			       "long, %lu was passed in",
			       type, (unsigned long)len, (unsigned long)size);
	return KRB5_PROG_ETYPE_NOSUPP;
    }
    ret = krb5_data_copy(&key->keyvalue, data, len);
    if(ret) {
	krb5_set_error_message(context, ret, N_("malloc: out of memory", ""));
	return ret;
    }
    key->keytype = type;

    return 0;
}