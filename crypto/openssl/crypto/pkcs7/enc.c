
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
	PKCS7 *p7;
	BIO *in;
	BIO *data,*p7bio;
	char buf[1024*4];
	int i;
	int nodetach=1;
	char *keyfile = NULL;
	const EVP_CIPHER *cipher=NULL;
	STACK_OF(X509) *recips=NULL;

	OpenSSL_add_all_algorithms();

	data=BIO_new(BIO_s_file());
	while(argc > 1)
		{
		if (strcmp(argv[1],"-nd") == 0)
			{
			nodetach=1;
			argv++; argc--;
			}
		else if ((strcmp(argv[1],"-c") == 0) && (argc >= 2)) {
			if(!(cipher = EVP_get_cipherbyname(argv[2]))) {
				fprintf(stderr, "Unknown cipher %s\n", argv[2]);
				goto err;
			}
			argc-=2;
			argv+=2;
		} else if ((strcmp(argv[1],"-k") == 0) && (argc >= 2)) {
			keyfile = argv[2];
			argc-=2;
			argv+=2;
			if (!(in=BIO_new_file(keyfile,"r"))) goto err;
			if (!(x509=PEM_read_bio_X509(in,NULL,NULL,NULL)))
				goto err;
			if(!recips) recips = sk_X509_new_null();
			sk_X509_push(recips, x509);
			BIO_free(in);
		} else break;
	}

	if(!recips) {
		fprintf(stderr, "No recipients\n");
		goto err;
	}

	if (!BIO_read_filename(data,argv[1])) goto err;

	p7=PKCS7_new();
#if 0
	BIO_reset(in);
	if ((pkey=PEM_read_bio_PrivateKey(in,NULL,NULL)) == NULL) goto err;
	BIO_free(in);
	PKCS7_set_type(p7,NID_pkcs7_signedAndEnveloped);
	 
	if (PKCS7_add_signature(p7,x509,pkey,EVP_sha1()) == NULL) goto err;
	/* we may want to add more */
	PKCS7_add_certificate(p7,x509);
#else
	PKCS7_set_type(p7,NID_pkcs7_enveloped);
#endif
	if(!cipher)	{
#ifndef OPENSSL_NO_DES
		cipher = EVP_des_ede3_cbc();
#else
		fprintf(stderr, "No cipher selected\n");
		goto err;
#endif
	}

	if (!PKCS7_set_cipher(p7,cipher)) goto err;
	for(i = 0; i < sk_X509_num(recips); i++) {
		if (!PKCS7_add_recipient(p7,sk_X509_value(recips, i))) goto err;
	}
	sk_X509_pop_free(recips, X509_free);

	/* Set the content of the signed to 'data' */
	/* PKCS7_content_new(p7,NID_pkcs7_data); not used in envelope */

	/* could be used, but not in this version :-)
	if (!nodetach) PKCS7_set_detached(p7,1);
	*/

	if ((p7bio=PKCS7_dataInit(p7,NULL)) == NULL) goto err;

	for (;;)
		{
		i=BIO_read(data,buf,sizeof(buf));
		if (i <= 0) break;
		BIO_write(p7bio,buf,i);
		}
	BIO_flush(p7bio);

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