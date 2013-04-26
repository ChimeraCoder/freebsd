
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
#include <time.h>
#include <errno.h>

#include "cryptlib.h"
#include <openssl/lhash.h>
#include <openssl/buffer.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

#ifndef OPENSSL_NO_STDIO

static int by_file_ctrl(X509_LOOKUP *ctx, int cmd, const char *argc,
	long argl, char **ret);
X509_LOOKUP_METHOD x509_file_lookup=
	{
	"Load file into cache",
	NULL,		/* new */
	NULL,		/* free */
	NULL, 		/* init */
	NULL,		/* shutdown */
	by_file_ctrl,	/* ctrl */
	NULL,		/* get_by_subject */
	NULL,		/* get_by_issuer_serial */
	NULL,		/* get_by_fingerprint */
	NULL,		/* get_by_alias */
	};

X509_LOOKUP_METHOD *X509_LOOKUP_file(void)
	{
	return(&x509_file_lookup);
	}

static int by_file_ctrl(X509_LOOKUP *ctx, int cmd, const char *argp, long argl,
	     char **ret)
	{
	int ok=0;
	char *file;

	switch (cmd)
		{
	case X509_L_FILE_LOAD:
		if (argl == X509_FILETYPE_DEFAULT)
			{
			file = (char *)getenv(X509_get_default_cert_file_env());
			if (file)
				ok = (X509_load_cert_crl_file(ctx,file,
					      X509_FILETYPE_PEM) != 0);

			else
				ok = (X509_load_cert_crl_file(ctx,X509_get_default_cert_file(),
					      X509_FILETYPE_PEM) != 0);

			if (!ok)
				{
				X509err(X509_F_BY_FILE_CTRL,X509_R_LOADING_DEFAULTS);
				}
			}
		else
			{
			if(argl == X509_FILETYPE_PEM)
				ok = (X509_load_cert_crl_file(ctx,argp,
					X509_FILETYPE_PEM) != 0);
			else
				ok = (X509_load_cert_file(ctx,argp,(int)argl) != 0);
			}
		break;
		}
	return(ok);
	}

int X509_load_cert_file(X509_LOOKUP *ctx, const char *file, int type)
	{
	int ret=0;
	BIO *in=NULL;
	int i,count=0;
	X509 *x=NULL;

	if (file == NULL) return(1);
	in=BIO_new(BIO_s_file_internal());

	if ((in == NULL) || (BIO_read_filename(in,file) <= 0))
		{
		X509err(X509_F_X509_LOAD_CERT_FILE,ERR_R_SYS_LIB);
		goto err;
		}

	if (type == X509_FILETYPE_PEM)
		{
		for (;;)
			{
			x=PEM_read_bio_X509_AUX(in,NULL,NULL,NULL);
			if (x == NULL)
				{
				if ((ERR_GET_REASON(ERR_peek_last_error()) ==
					PEM_R_NO_START_LINE) && (count > 0))
					{
					ERR_clear_error();
					break;
					}
				else
					{
					X509err(X509_F_X509_LOAD_CERT_FILE,
						ERR_R_PEM_LIB);
					goto err;
					}
				}
			i=X509_STORE_add_cert(ctx->store_ctx,x);
			if (!i) goto err;
			count++;
			X509_free(x);
			x=NULL;
			}
		ret=count;
		}
	else if (type == X509_FILETYPE_ASN1)
		{
		x=d2i_X509_bio(in,NULL);
		if (x == NULL)
			{
			X509err(X509_F_X509_LOAD_CERT_FILE,ERR_R_ASN1_LIB);
			goto err;
			}
		i=X509_STORE_add_cert(ctx->store_ctx,x);
		if (!i) goto err;
		ret=i;
		}
	else
		{
		X509err(X509_F_X509_LOAD_CERT_FILE,X509_R_BAD_X509_FILETYPE);
		goto err;
		}
err:
	if (x != NULL) X509_free(x);
	if (in != NULL) BIO_free(in);
	return(ret);
	}

int X509_load_crl_file(X509_LOOKUP *ctx, const char *file, int type)
	{
	int ret=0;
	BIO *in=NULL;
	int i,count=0;
	X509_CRL *x=NULL;

	if (file == NULL) return(1);
	in=BIO_new(BIO_s_file_internal());

	if ((in == NULL) || (BIO_read_filename(in,file) <= 0))
		{
		X509err(X509_F_X509_LOAD_CRL_FILE,ERR_R_SYS_LIB);
		goto err;
		}

	if (type == X509_FILETYPE_PEM)
		{
		for (;;)
			{
			x=PEM_read_bio_X509_CRL(in,NULL,NULL,NULL);
			if (x == NULL)
				{
				if ((ERR_GET_REASON(ERR_peek_last_error()) ==
					PEM_R_NO_START_LINE) && (count > 0))
					{
					ERR_clear_error();
					break;
					}
				else
					{
					X509err(X509_F_X509_LOAD_CRL_FILE,
						ERR_R_PEM_LIB);
					goto err;
					}
				}
			i=X509_STORE_add_crl(ctx->store_ctx,x);
			if (!i) goto err;
			count++;
			X509_CRL_free(x);
			x=NULL;
			}
		ret=count;
		}
	else if (type == X509_FILETYPE_ASN1)
		{
		x=d2i_X509_CRL_bio(in,NULL);
		if (x == NULL)
			{
			X509err(X509_F_X509_LOAD_CRL_FILE,ERR_R_ASN1_LIB);
			goto err;
			}
		i=X509_STORE_add_crl(ctx->store_ctx,x);
		if (!i) goto err;
		ret=i;
		}
	else
		{
		X509err(X509_F_X509_LOAD_CRL_FILE,X509_R_BAD_X509_FILETYPE);
		goto err;
		}
err:
	if (x != NULL) X509_CRL_free(x);
	if (in != NULL) BIO_free(in);
	return(ret);
	}

int X509_load_cert_crl_file(X509_LOOKUP *ctx, const char *file, int type)
{
	STACK_OF(X509_INFO) *inf;
	X509_INFO *itmp;
	BIO *in;
	int i, count = 0;
	if(type != X509_FILETYPE_PEM)
		return X509_load_cert_file(ctx, file, type);
	in = BIO_new_file(file, "r");
	if(!in) {
		X509err(X509_F_X509_LOAD_CERT_CRL_FILE,ERR_R_SYS_LIB);
		return 0;
	}
	inf = PEM_X509_INFO_read_bio(in, NULL, NULL, NULL);
	BIO_free(in);
	if(!inf) {
		X509err(X509_F_X509_LOAD_CERT_CRL_FILE,ERR_R_PEM_LIB);
		return 0;
	}
	for(i = 0; i < sk_X509_INFO_num(inf); i++) {
		itmp = sk_X509_INFO_value(inf, i);
		if(itmp->x509) {
			X509_STORE_add_cert(ctx->store_ctx, itmp->x509);
			count++;
		}
		if(itmp->crl) {
			X509_STORE_add_crl(ctx->store_ctx, itmp->crl);
			count++;
		}
	}
	sk_X509_INFO_pop_free(inf, X509_INFO_free);
	return count;
}


#endif /* OPENSSL_NO_STDIO */