
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

#include "ssl_locl.h"
#ifndef OPENSSL_NO_SSL2
#include <stdio.h>

int ssl2_enc_init(SSL *s, int client)
	{
	/* Max number of bytes needed */
	EVP_CIPHER_CTX *rs,*ws;
	const EVP_CIPHER *c;
	const EVP_MD *md;
	int num;

	if (!ssl_cipher_get_evp(s->session,&c,&md,NULL,NULL,NULL))
		{
		ssl2_return_error(s,SSL2_PE_NO_CIPHER);
		SSLerr(SSL_F_SSL2_ENC_INIT,SSL_R_PROBLEMS_MAPPING_CIPHER_FUNCTIONS);
		return(0);
		}
	ssl_replace_hash(&s->read_hash,md);
	ssl_replace_hash(&s->write_hash,md);

	if ((s->enc_read_ctx == NULL) &&
		((s->enc_read_ctx=(EVP_CIPHER_CTX *)
		OPENSSL_malloc(sizeof(EVP_CIPHER_CTX))) == NULL))
		goto err;

	/* make sure it's intialized in case the malloc for enc_write_ctx fails
	 * and we exit with an error */
	rs= s->enc_read_ctx;
	EVP_CIPHER_CTX_init(rs);

	if ((s->enc_write_ctx == NULL) &&
		((s->enc_write_ctx=(EVP_CIPHER_CTX *)
		OPENSSL_malloc(sizeof(EVP_CIPHER_CTX))) == NULL))
		goto err;

	ws= s->enc_write_ctx;
	EVP_CIPHER_CTX_init(ws);

	num=c->key_len;
	s->s2->key_material_length=num*2;
	OPENSSL_assert(s->s2->key_material_length <= sizeof s->s2->key_material);

	if (ssl2_generate_key_material(s) <= 0)
		return 0;

	OPENSSL_assert(c->iv_len <= (int)sizeof(s->session->key_arg));
	EVP_EncryptInit_ex(ws,c,NULL,&(s->s2->key_material[(client)?num:0]),
		s->session->key_arg);
	EVP_DecryptInit_ex(rs,c,NULL,&(s->s2->key_material[(client)?0:num]),
		s->session->key_arg);
	s->s2->read_key=  &(s->s2->key_material[(client)?0:num]);
	s->s2->write_key= &(s->s2->key_material[(client)?num:0]);
	return(1);
err:
	SSLerr(SSL_F_SSL2_ENC_INIT,ERR_R_MALLOC_FAILURE);
	return(0);
	}

/* read/writes from s->s2->mac_data using length for encrypt and 
 * decrypt.  It sets s->s2->padding and s->[rw]length
 * if we are encrypting */
void ssl2_enc(SSL *s, int send)
	{
	EVP_CIPHER_CTX *ds;
	unsigned long l;
	int bs;

	if (send)
		{
		ds=s->enc_write_ctx;
		l=s->s2->wlength;
		}
	else
		{
		ds=s->enc_read_ctx;
		l=s->s2->rlength;
		}

	/* check for NULL cipher */
	if (ds == NULL) return;


	bs=ds->cipher->block_size;
	/* This should be using (bs-1) and bs instead of 7 and 8, but
	 * what the hell. */
	if (bs == 8)
		l=(l+7)/8*8;

	EVP_Cipher(ds,s->s2->mac_data,s->s2->mac_data,l);
	}

void ssl2_mac(SSL *s, unsigned char *md, int send)
	{
	EVP_MD_CTX c;
	unsigned char sequence[4],*p,*sec,*act;
	unsigned long seq;
	unsigned int len;

	if (send)
		{
		seq=s->s2->write_sequence;
		sec=s->s2->write_key;
		len=s->s2->wact_data_length;
		act=s->s2->wact_data;
		}
	else
		{
		seq=s->s2->read_sequence;
		sec=s->s2->read_key;
		len=s->s2->ract_data_length;
		act=s->s2->ract_data;
		}

	p= &(sequence[0]);
	l2n(seq,p);

	/* There has to be a MAC algorithm. */
	EVP_MD_CTX_init(&c);
	EVP_MD_CTX_copy(&c, s->read_hash);
	EVP_DigestUpdate(&c,sec,
		EVP_CIPHER_CTX_key_length(s->enc_read_ctx));
	EVP_DigestUpdate(&c,act,len); 
	/* the above line also does the pad data */
	EVP_DigestUpdate(&c,sequence,4); 
	EVP_DigestFinal_ex(&c,md,NULL);
	EVP_MD_CTX_cleanup(&c);
	}
#else /* !OPENSSL_NO_SSL2 */

# if PEDANTIC
static void *dummy=&dummy;
# endif

#endif