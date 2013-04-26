
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

#include <stdio.h>
#include "ssl_locl.h"
#ifndef OPENSSL_NO_COMP
#include <openssl/comp.h>
#endif
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/md5.h>
#include <openssl/rand.h>
#ifdef KSSL_DEBUG
#include <openssl/des.h>
#endif

/* dtls1_enc encrypts/decrypts the record in |s->wrec| / |s->rrec|, respectively.
 *
 * Returns:
 *   0: (in non-constant time) if the record is publically invalid (i.e. too
 *       short etc).
 *   1: if the record's padding is valid / the encryption was successful.
 *   -1: if the record's padding/AEAD-authenticator is invalid or, if sending,
 *       an internal error occured. */
int dtls1_enc(SSL *s, int send)
	{
	SSL3_RECORD *rec;
	EVP_CIPHER_CTX *ds;
	unsigned long l;
	int bs,i,j,k,mac_size=0;
	const EVP_CIPHER *enc;

	if (send)
		{
		if (EVP_MD_CTX_md(s->write_hash))
			{
			mac_size=EVP_MD_CTX_size(s->write_hash);
			if (mac_size < 0)
				return -1;
			}
		ds=s->enc_write_ctx;
		rec= &(s->s3->wrec);
		if (s->enc_write_ctx == NULL)
			enc=NULL;
		else
			{
			enc=EVP_CIPHER_CTX_cipher(s->enc_write_ctx);
			if ( rec->data != rec->input)
				/* we can't write into the input stream */
				fprintf(stderr, "%s:%d: rec->data != rec->input\n",
					__FILE__, __LINE__);
			else if ( EVP_CIPHER_block_size(ds->cipher) > 1)
				{
				if (RAND_bytes(rec->input, EVP_CIPHER_block_size(ds->cipher)) <= 0)
					return -1;
				}
			}
		}
	else
		{
		if (EVP_MD_CTX_md(s->read_hash))
			{
			mac_size=EVP_MD_CTX_size(s->read_hash);
			OPENSSL_assert(mac_size >= 0);
			}
		ds=s->enc_read_ctx;
		rec= &(s->s3->rrec);
		if (s->enc_read_ctx == NULL)
			enc=NULL;
		else
			enc=EVP_CIPHER_CTX_cipher(s->enc_read_ctx);
		}

#ifdef KSSL_DEBUG
	printf("dtls1_enc(%d)\n", send);
#endif    /* KSSL_DEBUG */

	if ((s->session == NULL) || (ds == NULL) ||
		(enc == NULL))
		{
		memmove(rec->data,rec->input,rec->length);
		rec->input=rec->data;
		}
	else
		{
		l=rec->length;
		bs=EVP_CIPHER_block_size(ds->cipher);

		if ((bs != 1) && send)
			{
			i=bs-((int)l%bs);

			/* Add weird padding of upto 256 bytes */

			/* we need to add 'i' padding bytes of value j */
			j=i-1;
			if (s->options & SSL_OP_TLS_BLOCK_PADDING_BUG)
				{
				if (s->s3->flags & TLS1_FLAGS_TLS_PADDING_BUG)
					j++;
				}
			for (k=(int)l; k<(int)(l+i); k++)
				rec->input[k]=j;
			l+=i;
			rec->length+=i;
			}

#ifdef KSSL_DEBUG
		{
                unsigned long ui;
		printf("EVP_Cipher(ds=%p,rec->data=%p,rec->input=%p,l=%ld) ==>\n",
                        ds,rec->data,rec->input,l);
		printf("\tEVP_CIPHER_CTX: %d buf_len, %d key_len [%d %d], %d iv_len\n",
                        ds->buf_len, ds->cipher->key_len,
                        DES_KEY_SZ, DES_SCHEDULE_SZ,
                        ds->cipher->iv_len);
		printf("\t\tIV: ");
		for (i=0; i<ds->cipher->iv_len; i++) printf("%02X", ds->iv[i]);
		printf("\n");
		printf("\trec->input=");
		for (ui=0; ui<l; ui++) printf(" %02x", rec->input[ui]);
		printf("\n");
		}
#endif	/* KSSL_DEBUG */

		if (!send)
			{
			if (l == 0 || l%bs != 0)
				return 0;
			}
		
		EVP_Cipher(ds,rec->data,rec->input,l);

#ifdef KSSL_DEBUG
		{
                unsigned long i;
                printf("\trec->data=");
		for (i=0; i<l; i++)
                        printf(" %02x", rec->data[i]);  printf("\n");
                }
#endif	/* KSSL_DEBUG */

		if ((bs != 1) && !send)
			return tls1_cbc_remove_padding(s, rec, bs, mac_size);
		}
	return(1);
	}