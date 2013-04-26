
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
#include <string.h>
#include <openssl/bio.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/err.h>

int main(argc,argv)
int argc;
char *argv[];
	{
	X509 *x509;
	EVP_PKEY *pkey;
	PKCS7 *p7;
	PKCS7_SIGNER_INFO *si;
	BIO *in;
	BIO *data,*p7bio;
	char buf[1024*4];
	int i;
	int nodetach=0;

#ifndef OPENSSL_NO_MD2
	EVP_add_digest(EVP_md2());
#endif
#ifndef OPENSSL_NO_MD5
	EVP_add_digest(EVP_md5());
#endif
#ifndef OPENSSL_NO_SHA1
	EVP_add_digest(EVP_sha1());
#endif
#ifndef OPENSSL_NO_MDC2
	EVP_add_digest(EVP_mdc2());
#endif

	data=BIO_new(BIO_s_file());
again:
	if (argc > 1)
		{
		if (strcmp(argv[1],"-nd") == 0)
			{
			nodetach=1;
			argv++; argc--;
			goto again;
			}
		if (!BIO_read_filename(data,argv[1]))
			goto err;
		}
	else
		BIO_set_fp(data,stdin,BIO_NOCLOSE);

	if ((in=BIO_new_file("server.pem","r")) == NULL) goto err;
	if ((x509=PEM_read_bio_X509(in,NULL,NULL,NULL)) == NULL) goto err;
	BIO_reset(in);
	if ((pkey=PEM_read_bio_PrivateKey(in,NULL,NULL,NULL)) == NULL) goto err;
	BIO_free(in);

	p7=PKCS7_new();
	PKCS7_set_type(p7,NID_pkcs7_signed);
	 
	si=PKCS7_add_signature(p7,x509,pkey,EVP_sha1());
	if (si == NULL) goto err;

	/* If you do this then you get signing time automatically added */
	PKCS7_add_signed_attribute(si, NID_pkcs9_contentType, V_ASN1_OBJECT,
						OBJ_nid2obj(NID_pkcs7_data));

	/* we may want to add more */
	PKCS7_add_certificate(p7,x509);

	/* Set the content of the signed to 'data' */
	PKCS7_content_new(p7,NID_pkcs7_data);

	if (!nodetach)
		PKCS7_set_detached(p7,1);

	if ((p7bio=PKCS7_dataInit(p7,NULL)) == NULL) goto err;

	for (;;)
		{
		i=BIO_read(data,buf,sizeof(buf));
		if (i <= 0) break;
		BIO_write(p7bio,buf,i);
		}

	if (!PKCS7_dataFinal(p7,p7bio)) goto err;
	BIO_free(p7bio);

	PEM_write_PKCS7(stdout,p7);
	PKCS7_free(p7);

	exit(0);
err:
	ERR_load_crypto_strings();
	ERR_print_errors_fp(stderr);
	exit(1);
	}