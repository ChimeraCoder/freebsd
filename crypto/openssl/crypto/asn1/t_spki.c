
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
#include "cryptlib.h"
#include <openssl/x509.h>
#include <openssl/asn1.h>
#ifndef OPENSSL_NO_RSA
#include <openssl/rsa.h>
#endif
#ifndef OPENSSL_NO_DSA
#include <openssl/dsa.h>
#endif
#include <openssl/bn.h>

/* Print out an SPKI */

int NETSCAPE_SPKI_print(BIO *out, NETSCAPE_SPKI *spki)
{
	EVP_PKEY *pkey;
	ASN1_IA5STRING *chal;
	int i, n;
	char *s;
	BIO_printf(out, "Netscape SPKI:\n");
	i=OBJ_obj2nid(spki->spkac->pubkey->algor->algorithm);
	BIO_printf(out,"  Public Key Algorithm: %s\n",
				(i == NID_undef)?"UNKNOWN":OBJ_nid2ln(i));
	pkey = X509_PUBKEY_get(spki->spkac->pubkey);
	if(!pkey) BIO_printf(out, "  Unable to load public key\n");
	else
		{
		EVP_PKEY_print_public(out, pkey, 4, NULL);
		EVP_PKEY_free(pkey);
		}
	chal = spki->spkac->challenge;
	if(chal->length)
		BIO_printf(out, "  Challenge String: %s\n", chal->data);
	i=OBJ_obj2nid(spki->sig_algor->algorithm);
	BIO_printf(out,"  Signature Algorithm: %s",
				(i == NID_undef)?"UNKNOWN":OBJ_nid2ln(i));

	n=spki->signature->length;
	s=(char *)spki->signature->data;
	for (i=0; i<n; i++)
		{
		if ((i%18) == 0) BIO_write(out,"\n      ",7);
		BIO_printf(out,"%02x%s",(unsigned char)s[i],
						((i+1) == n)?"":":");
		}
	BIO_write(out,"\n",1);
	return 1;
}