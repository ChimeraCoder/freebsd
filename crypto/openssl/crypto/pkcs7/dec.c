
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
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/asn1.h>

int verify_callback(int ok, X509_STORE_CTX *ctx);

BIO *bio_err=NULL;

int main(argc,argv)
int argc;
char *argv[];
	{
	char *keyfile=NULL;
	BIO *in;
	EVP_PKEY *pkey;
	X509 *x509;
	PKCS7 *p7;
	PKCS7_SIGNER_INFO *si;
	X509_STORE_CTX cert_ctx;
	X509_STORE *cert_store=NULL;
	BIO *data,*detached=NULL,*p7bio=NULL;
	char buf[1024*4];
	unsigned char *pp;
	int i,printit=0;
	STACK_OF(PKCS7_SIGNER_INFO) *sk;

	OpenSSL_add_all_algorithms();
	bio_err=BIO_new_fp(stderr,BIO_NOCLOSE);

	data=BIO_new(BIO_s_file());
	pp=NULL;
	while (argc > 1)
		{
		argc--;
		argv++;
		if (strcmp(argv[0],"-p") == 0)
			{
			printit=1;
			}
		else if ((strcmp(argv[0],"-k") == 0) && (argc >= 2)) {
			keyfile = argv[1];
			argc-=1;
			argv+=1;
		} else if ((strcmp(argv[0],"-d") == 0) && (argc >= 2))
			{
			detached=BIO_new(BIO_s_file());
			if (!BIO_read_filename(detached,argv[1]))
				goto err;
			argc-=1;
			argv+=1;
			}
		else break;
		}

	 if (!BIO_read_filename(data,argv[0])) goto err; 

	if(!keyfile) {
		fprintf(stderr, "No private key file specified\n");
		goto err;
	}

        if ((in=BIO_new_file(keyfile,"r")) == NULL) goto err;
        if ((x509=PEM_read_bio_X509(in,NULL,NULL,NULL)) == NULL) goto err;
        BIO_reset(in);
        if ((pkey=PEM_read_bio_PrivateKey(in,NULL,NULL,NULL)) == NULL)
		goto err;
        BIO_free(in);

	if (pp == NULL)
		BIO_set_fp(data,stdin,BIO_NOCLOSE);


	/* Load the PKCS7 object from a file */
	if ((p7=PEM_read_bio_PKCS7(data,NULL,NULL,NULL)) == NULL) goto err;



	/* This stuff is being setup for certificate verification.
	 * When using SSL, it could be replaced with a 
	 * cert_stre=SSL_CTX_get_cert_store(ssl_ctx); */
	cert_store=X509_STORE_new();
	X509_STORE_set_default_paths(cert_store);
	X509_STORE_load_locations(cert_store,NULL,"../../certs");
	X509_STORE_set_verify_cb_func(cert_store,verify_callback);

	ERR_clear_error();

	/* We need to process the data */
	/* We cannot support detached encryption */
	p7bio=PKCS7_dataDecode(p7,pkey,detached,x509);

	if (p7bio == NULL)
		{
		printf("problems decoding\n");
		goto err;
		}

	/* We now have to 'read' from p7bio to calculate digests etc. */
	for (;;)
		{
		i=BIO_read(p7bio,buf,sizeof(buf));
		/* print it? */
		if (i <= 0) break;
		fwrite(buf,1, i, stdout);
		}

	/* We can now verify signatures */
	sk=PKCS7_get_signer_info(p7);
	if (sk == NULL)
		{
		fprintf(stderr, "there are no signatures on this data\n");
		}
	else
		{
		/* Ok, first we need to, for each subject entry,
		 * see if we can verify */
		ERR_clear_error();
		for (i=0; i<sk_PKCS7_SIGNER_INFO_num(sk); i++)
			{
			si=sk_PKCS7_SIGNER_INFO_value(sk,i);
			i=PKCS7_dataVerify(cert_store,&cert_ctx,p7bio,p7,si);
			if (i <= 0)
				goto err;
			else
				fprintf(stderr,"Signature verified\n");
			}
		}
	X509_STORE_free(cert_store);

	exit(0);
err:
	ERR_load_crypto_strings();
	ERR_print_errors_fp(stderr);
	exit(1);
	}

/* should be X509 * but we can just have them as char *. */
int verify_callback(int ok, X509_STORE_CTX *ctx)
	{
	char buf[256];
	X509 *err_cert;
	int err,depth;

	err_cert=X509_STORE_CTX_get_current_cert(ctx);
	err=	X509_STORE_CTX_get_error(ctx);
	depth=	X509_STORE_CTX_get_error_depth(ctx);

	X509_NAME_oneline(X509_get_subject_name(err_cert),buf,256);
	BIO_printf(bio_err,"depth=%d %s\n",depth,buf);
	if (!ok)
		{
		BIO_printf(bio_err,"verify error:num=%d:%s\n",err,
			X509_verify_cert_error_string(err));
		if (depth < 6)
			{
			ok=1;
			X509_STORE_CTX_set_error(ctx,X509_V_OK);
			}
		else
			{
			ok=0;
			X509_STORE_CTX_set_error(ctx,X509_V_ERR_CERT_CHAIN_TOO_LONG);
			}
		}
	switch (ctx->error)
		{
	case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
		X509_NAME_oneline(X509_get_issuer_name(ctx->current_cert),buf,256);
		BIO_printf(bio_err,"issuer= %s\n",buf);
		break;
	case X509_V_ERR_CERT_NOT_YET_VALID:
	case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
		BIO_printf(bio_err,"notBefore=");
		ASN1_UTCTIME_print(bio_err,X509_get_notBefore(ctx->current_cert));
		BIO_printf(bio_err,"\n");
		break;
	case X509_V_ERR_CERT_HAS_EXPIRED:
	case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
		BIO_printf(bio_err,"notAfter=");
		ASN1_UTCTIME_print(bio_err,X509_get_notAfter(ctx->current_cert));
		BIO_printf(bio_err,"\n");
		break;
		}
	BIO_printf(bio_err,"verify return:%d\n",ok);
	return(ok);
	}