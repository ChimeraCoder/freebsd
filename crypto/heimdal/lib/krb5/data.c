
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
 * Reset the (potentially uninitalized) krb5_data structure.
 *
 * @param p krb5_data to reset.
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_data_zero(krb5_data *p)
{
    p->length = 0;
    p->data   = NULL;
}

/**
 * Free the content of krb5_data structure, its ok to free a zeroed
 * structure (with memset() or krb5_data_zero()). When done, the
 * structure will be zeroed. The same function is called
 * krb5_free_data_contents() in MIT Kerberos.
 *
 * @param p krb5_data to free.
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_data_free(krb5_data *p)
{
    if(p->data != NULL)
	free(p->data);
    krb5_data_zero(p);
}

/**
 * Free krb5_data (and its content).
 *
 * @param context Kerberos 5 context.
 * @param p krb5_data to free.
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION void KRB5_LIB_CALL
krb5_free_data(krb5_context context,
	       krb5_data *p)
{
    krb5_data_free(p);
    free(p);
}

/**
 * Allocate data of and krb5_data.
 *
 * @param p krb5_data to allocate.
 * @param len size to allocate.
 *
 * @return Returns 0 to indicate success. Otherwise an kerberos et
 * error code is returned.
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_data_alloc(krb5_data *p, int len)
{
    p->data = malloc(len);
    if(len && p->data == NULL)
	return ENOMEM;
    p->length = len;
    return 0;
}

/**
 * Grow (or shrink) the content of krb5_data to a new size.
 *
 * @param p krb5_data to free.
 * @param len new size.
 *
 * @return Returns 0 to indicate success. Otherwise an kerberos et
 * error code is returned.
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_data_realloc(krb5_data *p, int len)
{
    void *tmp;
    tmp = realloc(p->data, len);
    if(len && !tmp)
	return ENOMEM;
    p->data = tmp;
    p->length = len;
    return 0;
}

/**
 * Copy the data of len into the krb5_data.
 *
 * @param p krb5_data to copy into.
 * @param data data to copy..
 * @param len new size.
 *
 * @return Returns 0 to indicate success. Otherwise an kerberos et
 * error code is returned.
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_data_copy(krb5_data *p, const void *data, size_t len)
{
    if (len) {
	if(krb5_data_alloc(p, len))
	    return ENOMEM;
	memmove(p->data, data, len);
    } else
	p->data = NULL;
    p->length = len;
    return 0;
}

/**
 * Copy the data into a newly allocated krb5_data.
 *
 * @param context Kerberos 5 context.
 * @param indata the krb5_data data to copy
 * @param outdata new krb5_date to copy too. Free with krb5_free_data().
 *
 * @return Returns 0 to indicate success. Otherwise an kerberos et
 * error code is returned.
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION krb5_error_code KRB5_LIB_CALL
krb5_copy_data(krb5_context context,
	       const krb5_data *indata,
	       krb5_data **outdata)
{
    krb5_error_code ret;
    ALLOC(*outdata, 1);
    if(*outdata == NULL) {
	krb5_set_error_message(context, ENOMEM, "malloc: out of memory");
	return ENOMEM;
    }
    ret = der_copy_octet_string(indata, *outdata);
    if(ret) {
	krb5_clear_error_message (context);
	free(*outdata);
	*outdata = NULL;
    }
    return ret;
}

/**
 * Compare to data.
 *
 * @param data1 krb5_data to compare
 * @param data2 krb5_data to compare
 *
 * @return return the same way as memcmp(), useful when sorting.
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION int KRB5_LIB_CALL
krb5_data_cmp(const krb5_data *data1, const krb5_data *data2)
{
    if (data1->length != data2->length)
	return data1->length - data2->length;
    return memcmp(data1->data, data2->data, data1->length);
}

/**
 * Compare to data not exposing timing information from the checksum data
 *
 * @param data1 krb5_data to compare
 * @param data2 krb5_data to compare
 *
 * @return returns zero for same data, otherwise non zero.
 *
 * @ingroup krb5
 */

KRB5_LIB_FUNCTION int KRB5_LIB_CALL
krb5_data_ct_cmp(const krb5_data *data1, const krb5_data *data2)
{
    if (data1->length != data2->length)
	return data1->length - data2->length;
    return ct_memcmp(data1->data, data2->data, data1->length);
}