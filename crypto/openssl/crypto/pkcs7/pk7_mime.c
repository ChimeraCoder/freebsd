
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
#include <ctype.h>
#include "cryptlib.h"
#include <openssl/rand.h>
#include <openssl/x509.h>
#include <openssl/asn1.h>

/* PKCS#7 wrappers round generalised stream and MIME routines */

int i2d_PKCS7_bio_stream(BIO *out, PKCS7 *p7, BIO *in, int flags)
	{
	return i2d_ASN1_bio_stream(out, (ASN1_VALUE *)p7, in, flags,
					ASN1_ITEM_rptr(PKCS7));
	}

int PEM_write_bio_PKCS7_stream(BIO *out, PKCS7 *p7, BIO *in, int flags)
	{
	return PEM_write_bio_ASN1_stream(out, (ASN1_VALUE *) p7, in, flags,
						"PKCS7",
						ASN1_ITEM_rptr(PKCS7));
	}

int SMIME_write_PKCS7(BIO *bio, PKCS7 *p7, BIO *data, int flags)
	{
	STACK_OF(X509_ALGOR) *mdalgs;
	int ctype_nid = OBJ_obj2nid(p7->type);
	if (ctype_nid == NID_pkcs7_signed)
		mdalgs = p7->d.sign->md_algs;
	else
		mdalgs = NULL;

	flags ^= SMIME_OLDMIME;


	return SMIME_write_ASN1(bio, (ASN1_VALUE *)p7, data, flags,
					ctype_nid, NID_undef, mdalgs,
					ASN1_ITEM_rptr(PKCS7));	
	}

PKCS7 *SMIME_read_PKCS7(BIO *bio, BIO **bcont)
	{
	return (PKCS7 *)SMIME_read_ASN1(bio, bcont, ASN1_ITEM_rptr(PKCS7));
	}