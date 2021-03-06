
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

#include "rsh_locl.h"
RCSID("$Id$");

#if defined(KRB5)

#ifdef KRB5
int key_usage = 1026;

void *ivec_in[2];
void *ivec_out[2];

void
init_ivecs(int client, int have_errsock)
{
    size_t blocksize;

    krb5_crypto_getblocksize(context, crypto, &blocksize);

    ivec_in[0] = malloc(blocksize);
    memset(ivec_in[0], client, blocksize);

    if(have_errsock) {
	ivec_in[1] = malloc(blocksize);
	memset(ivec_in[1], 2 | client, blocksize);
    } else
	ivec_in[1] = ivec_in[0];

    ivec_out[0] = malloc(blocksize);
    memset(ivec_out[0], !client, blocksize);

    if(have_errsock) {
	ivec_out[1] = malloc(blocksize);
	memset(ivec_out[1], 2 | !client, blocksize);
    } else
	ivec_out[1] = ivec_out[0];
}
#endif


ssize_t
do_read (int fd, void *buf, size_t sz, void *ivec)
{
    if (do_encrypt) {
#ifdef KRB5
        if(auth_method == AUTH_KRB5) {
	    krb5_error_code ret;
	    uint32_t len, outer_len;
	    int status;
	    krb5_data data;
	    void *edata;

	    ret = krb5_net_read (context, &fd, &len, 4);
	    if (ret <= 0)
		return ret;
	    len = ntohl(len);
	    if (len > sz)
		abort ();
	    /* ivec will be non null for protocol version 2 */
	    if(ivec != NULL)
		outer_len = krb5_get_wrapped_length (context, crypto, len + 4);
	    else
		outer_len = krb5_get_wrapped_length (context, crypto, len);
	    edata = malloc (outer_len);
	    if (edata == NULL)
		errx (1, "malloc: cannot allocate %u bytes", outer_len);
	    ret = krb5_net_read (context, &fd, edata, outer_len);
	    if (ret <= 0) {
		free(edata);
		return ret;
	    }

	    status = krb5_decrypt_ivec(context, crypto, key_usage,
				       edata, outer_len, &data, ivec);
	    free (edata);

	    if (status)
		krb5_err (context, 1, status, "decrypting data");
	    if(ivec != NULL) {
		unsigned long l;
		if(data.length < len + 4)
		    errx (1, "data received is too short");
		_krb5_get_int(data.data, &l, 4);
		if(l != len)
		    errx (1, "inconsistency in received data");
		memcpy (buf, (unsigned char *)data.data+4, len);
	    } else
		memcpy (buf, data.data, len);
	    krb5_data_free (&data);
	    return len;
	} else
#endif /* KRB5 */
	    abort ();
    } else
	return read (fd, buf, sz);
}

ssize_t
do_write (int fd, void *buf, size_t sz, void *ivec)
{
    if (do_encrypt) {
#ifdef KRB5
	if(auth_method == AUTH_KRB5) {
	    krb5_error_code status;
	    krb5_data data;
	    unsigned char len[4];
	    int ret;

	    _krb5_put_int(len, sz, 4);
	    if(ivec != NULL) {
		unsigned char *tmp = malloc(sz + 4);
		if(tmp == NULL)
		    err(1, "malloc");
		_krb5_put_int(tmp, sz, 4);
		memcpy(tmp + 4, buf, sz);
		status = krb5_encrypt_ivec(context, crypto, key_usage,
					   tmp, sz + 4, &data, ivec);
		free(tmp);
	    } else
		status = krb5_encrypt_ivec(context, crypto, key_usage,
					   buf, sz, &data, ivec);

	    if (status)
		krb5_err(context, 1, status, "encrypting data");

	    ret = krb5_net_write (context, &fd, len, 4);
	    if (ret != 4)
		return ret;
	    ret = krb5_net_write (context, &fd, data.data, data.length);
	    if (ret != data.length)
		return ret;
	    free (data.data);
	    return sz;
	} else
#endif /* KRB5 */
	    abort();
    } else
	return write (fd, buf, sz);
}
#endif /* KRB5 */