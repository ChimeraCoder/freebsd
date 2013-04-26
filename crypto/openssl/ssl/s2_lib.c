
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
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/md5.h>

const char ssl2_version_str[]="SSLv2" OPENSSL_VERSION_PTEXT;

#define SSL2_NUM_CIPHERS (sizeof(ssl2_ciphers)/sizeof(SSL_CIPHER))

/* list of available SSLv2 ciphers (sorted by id) */
OPENSSL_GLOBAL const SSL_CIPHER ssl2_ciphers[]={
#if 0
/* NULL_WITH_MD5 v3 */
	{
	1,
	SSL2_TXT_NULL_WITH_MD5,
	SSL2_CK_NULL_WITH_MD5,
	SSL_kRSA,
	SSL_aRSA,
	SSL_eNULL,
	SSL_MD5,
	SSL_SSLV2,
	SSL_EXPORT|SSL_EXP40|SSL_STRONG_NONE,
	0,
	0,
	0,
	},
#endif

/* RC4_128_WITH_MD5 */
	{
	1,
	SSL2_TXT_RC4_128_WITH_MD5,
	SSL2_CK_RC4_128_WITH_MD5,
	SSL_kRSA,
	SSL_aRSA,
	SSL_RC4,
	SSL_MD5,
	SSL_SSLV2,
	SSL_NOT_EXP|SSL_MEDIUM,
	0,
	128,
	128,
	},

/* RC4_128_EXPORT40_WITH_MD5 */
	{
	1,
	SSL2_TXT_RC4_128_EXPORT40_WITH_MD5,
	SSL2_CK_RC4_128_EXPORT40_WITH_MD5,
	SSL_kRSA,
	SSL_aRSA,
	SSL_RC4,
	SSL_MD5,
	SSL_SSLV2,
	SSL_EXPORT|SSL_EXP40,
	SSL2_CF_5_BYTE_ENC,
	40,
	128,
	},

/* RC2_128_CBC_WITH_MD5 */
	{
	1,
	SSL2_TXT_RC2_128_CBC_WITH_MD5,
	SSL2_CK_RC2_128_CBC_WITH_MD5,
	SSL_kRSA,
	SSL_aRSA,
	SSL_RC2,
	SSL_MD5,
	SSL_SSLV2,
	SSL_NOT_EXP|SSL_MEDIUM,
	0,
	128,
	128,
	},

/* RC2_128_CBC_EXPORT40_WITH_MD5 */
	{
	1,
	SSL2_TXT_RC2_128_CBC_EXPORT40_WITH_MD5,
	SSL2_CK_RC2_128_CBC_EXPORT40_WITH_MD5,
	SSL_kRSA,
	SSL_aRSA,
	SSL_RC2,
	SSL_MD5,
	SSL_SSLV2,
	SSL_EXPORT|SSL_EXP40,
	SSL2_CF_5_BYTE_ENC,
	40,
	128,
	},

#ifndef OPENSSL_NO_IDEA
/* IDEA_128_CBC_WITH_MD5 */
	{
	1,
	SSL2_TXT_IDEA_128_CBC_WITH_MD5,
	SSL2_CK_IDEA_128_CBC_WITH_MD5,
	SSL_kRSA,
	SSL_aRSA,
	SSL_IDEA,
	SSL_MD5,
	SSL_SSLV2,
	SSL_NOT_EXP|SSL_MEDIUM,
	0,
	128,
	128,
	},
#endif

/* DES_64_CBC_WITH_MD5 */
	{
	1,
	SSL2_TXT_DES_64_CBC_WITH_MD5,
	SSL2_CK_DES_64_CBC_WITH_MD5,
	SSL_kRSA,
	SSL_aRSA,
	SSL_DES,
	SSL_MD5,
	SSL_SSLV2,
	SSL_NOT_EXP|SSL_LOW,
	0,
	56,
	56,
	},

/* DES_192_EDE3_CBC_WITH_MD5 */
	{
	1,
	SSL2_TXT_DES_192_EDE3_CBC_WITH_MD5,
	SSL2_CK_DES_192_EDE3_CBC_WITH_MD5,
	SSL_kRSA,
	SSL_aRSA,
	SSL_3DES,
	SSL_MD5,
	SSL_SSLV2,
	SSL_NOT_EXP|SSL_HIGH,
	0,
	168,
	168,
	},

#if 0
/* RC4_64_WITH_MD5 */
	{
	1,
	SSL2_TXT_RC4_64_WITH_MD5,
	SSL2_CK_RC4_64_WITH_MD5,
	SSL_kRSA,
	SSL_aRSA,
	SSL_RC4,
	SSL_MD5,
	SSL_SSLV2,
	SSL_NOT_EXP|SSL_LOW,
	SSL2_CF_8_BYTE_ENC,
	64,
	64,
	},
#endif

#if 0
/* NULL SSLeay (testing) */
	{	
	0,
	SSL2_TXT_NULL,
	SSL2_CK_NULL,
	0,
	0,
	0,
	0,
	SSL_SSLV2,
	SSL_STRONG_NONE,
	0,
	0,
	0,
	},
#endif

/* end of list :-) */
	};

long ssl2_default_timeout(void)
	{
	return(300);
	}

int ssl2_num_ciphers(void)
	{
	return(SSL2_NUM_CIPHERS);
	}

const SSL_CIPHER *ssl2_get_cipher(unsigned int u)
	{
	if (u < SSL2_NUM_CIPHERS)
		return(&(ssl2_ciphers[SSL2_NUM_CIPHERS-1-u]));
	else
		return(NULL);
	}

int ssl2_pending(const SSL *s)
	{
	return SSL_in_init(s) ? 0 : s->s2->ract_data_length;
	}

int ssl2_new(SSL *s)
	{
	SSL2_STATE *s2;

	if ((s2=OPENSSL_malloc(sizeof *s2)) == NULL) goto err;
	memset(s2,0,sizeof *s2);

#if SSL2_MAX_RECORD_LENGTH_3_BYTE_HEADER + 3 > SSL2_MAX_RECORD_LENGTH_2_BYTE_HEADER + 2
#  error "assertion failed"
#endif

	if ((s2->rbuf=OPENSSL_malloc(
		SSL2_MAX_RECORD_LENGTH_2_BYTE_HEADER+2)) == NULL) goto err;
	/* wbuf needs one byte more because when using two-byte headers,
	 * we leave the first byte unused in do_ssl_write (s2_pkt.c) */
	if ((s2->wbuf=OPENSSL_malloc(
		SSL2_MAX_RECORD_LENGTH_2_BYTE_HEADER+3)) == NULL) goto err;
	s->s2=s2;

	ssl2_clear(s);
	return(1);
err:
	if (s2 != NULL)
		{
		if (s2->wbuf != NULL) OPENSSL_free(s2->wbuf);
		if (s2->rbuf != NULL) OPENSSL_free(s2->rbuf);
		OPENSSL_free(s2);
		}
	return(0);
	}

void ssl2_free(SSL *s)
	{
	SSL2_STATE *s2;

	if(s == NULL)
	    return;

	s2=s->s2;
	if (s2->rbuf != NULL) OPENSSL_free(s2->rbuf);
	if (s2->wbuf != NULL) OPENSSL_free(s2->wbuf);
	OPENSSL_cleanse(s2,sizeof *s2);
	OPENSSL_free(s2);
	s->s2=NULL;
	}

void ssl2_clear(SSL *s)
	{
	SSL2_STATE *s2;
	unsigned char *rbuf,*wbuf;

	s2=s->s2;

	rbuf=s2->rbuf;
	wbuf=s2->wbuf;

	memset(s2,0,sizeof *s2);

	s2->rbuf=rbuf;
	s2->wbuf=wbuf;
	s2->clear_text=1;
	s->packet=s2->rbuf;
	s->version=SSL2_VERSION;
	s->packet_length=0;
	}

long ssl2_ctrl(SSL *s, int cmd, long larg, void *parg)
	{
	int ret=0;

	switch(cmd)
		{
	case SSL_CTRL_GET_SESSION_REUSED:
		ret=s->hit;
		break;
	default:
		break;
		}
	return(ret);
	}

long ssl2_callback_ctrl(SSL *s, int cmd, void (*fp)(void))
	{
	return(0);
	}

long ssl2_ctx_ctrl(SSL_CTX *ctx, int cmd, long larg, void *parg)
	{
	return(0);
	}

long ssl2_ctx_callback_ctrl(SSL_CTX *ctx, int cmd, void (*fp)(void))
	{
	return(0);
	}

/* This function needs to check if the ciphers required are actually
 * available */
const SSL_CIPHER *ssl2_get_cipher_by_char(const unsigned char *p)
	{
	SSL_CIPHER c;
	const SSL_CIPHER *cp;
	unsigned long id;

	id=0x02000000L|((unsigned long)p[0]<<16L)|
		((unsigned long)p[1]<<8L)|(unsigned long)p[2];
	c.id=id;
	cp = OBJ_bsearch_ssl_cipher_id(&c, ssl2_ciphers, SSL2_NUM_CIPHERS);
	if ((cp == NULL) || (cp->valid == 0))
		return NULL;
	else
		return cp;
	}

int ssl2_put_cipher_by_char(const SSL_CIPHER *c, unsigned char *p)
	{
	long l;

	if (p != NULL)
		{
		l=c->id;
		if ((l & 0xff000000) != 0x02000000) return(0);
		p[0]=((unsigned char)(l>>16L))&0xFF;
		p[1]=((unsigned char)(l>> 8L))&0xFF;
		p[2]=((unsigned char)(l     ))&0xFF;
		}
	return(3);
	}

int ssl2_generate_key_material(SSL *s)
	{
	unsigned int i;
	EVP_MD_CTX ctx;
	unsigned char *km;
	unsigned char c='0';
	const EVP_MD *md5;
	int md_size;

	md5 = EVP_md5();

#ifdef CHARSET_EBCDIC
	c = os_toascii['0']; /* Must be an ASCII '0', not EBCDIC '0',
				see SSLv2 docu */
#endif
	EVP_MD_CTX_init(&ctx);
	km=s->s2->key_material;

 	if (s->session->master_key_length < 0 ||
			s->session->master_key_length > (int)sizeof(s->session->master_key))
 		{
 		SSLerr(SSL_F_SSL2_GENERATE_KEY_MATERIAL, ERR_R_INTERNAL_ERROR);
 		return 0;
 		}
	md_size = EVP_MD_size(md5);
	if (md_size < 0)
	    return 0;
	for (i=0; i<s->s2->key_material_length; i += md_size)
		{
		if (((km - s->s2->key_material) + md_size) >
				(int)sizeof(s->s2->key_material))
			{
			/* EVP_DigestFinal_ex() below would write beyond buffer */
			SSLerr(SSL_F_SSL2_GENERATE_KEY_MATERIAL, ERR_R_INTERNAL_ERROR);
			return 0;
			}

		EVP_DigestInit_ex(&ctx, md5, NULL);

		OPENSSL_assert(s->session->master_key_length >= 0
		    && s->session->master_key_length
		    < (int)sizeof(s->session->master_key));
		EVP_DigestUpdate(&ctx,s->session->master_key,s->session->master_key_length);
		EVP_DigestUpdate(&ctx,&c,1);
		c++;
		EVP_DigestUpdate(&ctx,s->s2->challenge,s->s2->challenge_length);
		EVP_DigestUpdate(&ctx,s->s2->conn_id,s->s2->conn_id_length);
		EVP_DigestFinal_ex(&ctx,km,NULL);
		km += md_size;
		}

	EVP_MD_CTX_cleanup(&ctx);
	return 1;
	}

void ssl2_return_error(SSL *s, int err)
	{
	if (!s->error)
		{
		s->error=3;
		s->error_code=err;

		ssl2_write_error(s);
		}
	}


void ssl2_write_error(SSL *s)
	{
	unsigned char buf[3];
	int i,error;

	buf[0]=SSL2_MT_ERROR;
	buf[1]=(s->error_code>>8)&0xff;
	buf[2]=(s->error_code)&0xff;

/*	state=s->rwstate;*/

	error=s->error; /* number of bytes left to write */
	s->error=0;
	OPENSSL_assert(error >= 0 && error <= (int)sizeof(buf));
	i=ssl2_write(s,&(buf[3-error]),error);

/*	if (i == error) s->rwstate=state; */

	if (i < 0)
		s->error=error;
	else
		{
		s->error=error-i;

		if (s->error == 0)
			if (s->msg_callback)
				s->msg_callback(1, s->version, 0, buf, 3, s, s->msg_callback_arg); /* ERROR */
		}
	}

int ssl2_shutdown(SSL *s)
	{
	s->shutdown=(SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);
	return(1);
	}
#else /* !OPENSSL_NO_SSL2 */

# if PEDANTIC
static void *dummy=&dummy;
# endif

#endif