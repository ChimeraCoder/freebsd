
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
#include <openssl/evp.h>
#include <openssl/asn1t.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

ASN1_SEQUENCE_enc(X509_CINF, enc, 0) = {
	ASN1_EXP_OPT(X509_CINF, version, ASN1_INTEGER, 0),
	ASN1_SIMPLE(X509_CINF, serialNumber, ASN1_INTEGER),
	ASN1_SIMPLE(X509_CINF, signature, X509_ALGOR),
	ASN1_SIMPLE(X509_CINF, issuer, X509_NAME),
	ASN1_SIMPLE(X509_CINF, validity, X509_VAL),
	ASN1_SIMPLE(X509_CINF, subject, X509_NAME),
	ASN1_SIMPLE(X509_CINF, key, X509_PUBKEY),
	ASN1_IMP_OPT(X509_CINF, issuerUID, ASN1_BIT_STRING, 1),
	ASN1_IMP_OPT(X509_CINF, subjectUID, ASN1_BIT_STRING, 2),
	ASN1_EXP_SEQUENCE_OF_OPT(X509_CINF, extensions, X509_EXTENSION, 3)
} ASN1_SEQUENCE_END_enc(X509_CINF, X509_CINF)

IMPLEMENT_ASN1_FUNCTIONS(X509_CINF)
/* X509 top level structure needs a bit of customisation */

extern void policy_cache_free(X509_POLICY_CACHE *cache);

static int x509_cb(int operation, ASN1_VALUE **pval, const ASN1_ITEM *it,
								void *exarg)
{
	X509 *ret = (X509 *)*pval;

	switch(operation) {

		case ASN1_OP_NEW_POST:
		ret->valid=0;
		ret->name = NULL;
		ret->ex_flags = 0;
		ret->ex_pathlen = -1;
		ret->skid = NULL;
		ret->akid = NULL;
#ifndef OPENSSL_NO_RFC3779
		ret->rfc3779_addr = NULL;
		ret->rfc3779_asid = NULL;
#endif
		ret->aux = NULL;
		ret->crldp = NULL;
		CRYPTO_new_ex_data(CRYPTO_EX_INDEX_X509, ret, &ret->ex_data);
		break;

		case ASN1_OP_D2I_POST:
		if (ret->name != NULL) OPENSSL_free(ret->name);
		ret->name=X509_NAME_oneline(ret->cert_info->subject,NULL,0);
		break;

		case ASN1_OP_FREE_POST:
		CRYPTO_free_ex_data(CRYPTO_EX_INDEX_X509, ret, &ret->ex_data);
		X509_CERT_AUX_free(ret->aux);
		ASN1_OCTET_STRING_free(ret->skid);
		AUTHORITY_KEYID_free(ret->akid);
		CRL_DIST_POINTS_free(ret->crldp);
		policy_cache_free(ret->policy_cache);
		GENERAL_NAMES_free(ret->altname);
		NAME_CONSTRAINTS_free(ret->nc);
#ifndef OPENSSL_NO_RFC3779
		sk_IPAddressFamily_pop_free(ret->rfc3779_addr, IPAddressFamily_free);
		ASIdentifiers_free(ret->rfc3779_asid);
#endif

		if (ret->name != NULL) OPENSSL_free(ret->name);
		break;

	}

	return 1;

}

ASN1_SEQUENCE_ref(X509, x509_cb, CRYPTO_LOCK_X509) = {
	ASN1_SIMPLE(X509, cert_info, X509_CINF),
	ASN1_SIMPLE(X509, sig_alg, X509_ALGOR),
	ASN1_SIMPLE(X509, signature, ASN1_BIT_STRING)
} ASN1_SEQUENCE_END_ref(X509, X509)

IMPLEMENT_ASN1_FUNCTIONS(X509)
IMPLEMENT_ASN1_DUP_FUNCTION(X509)

int X509_get_ex_new_index(long argl, void *argp, CRYPTO_EX_new *new_func,
	     CRYPTO_EX_dup *dup_func, CRYPTO_EX_free *free_func)
        {
	return CRYPTO_get_ex_new_index(CRYPTO_EX_INDEX_X509, argl, argp,
				new_func, dup_func, free_func);
        }

int X509_set_ex_data(X509 *r, int idx, void *arg)
	{
	return(CRYPTO_set_ex_data(&r->ex_data,idx,arg));
	}

void *X509_get_ex_data(X509 *r, int idx)
	{
	return(CRYPTO_get_ex_data(&r->ex_data,idx));
	}

/* X509_AUX ASN1 routines. X509_AUX is the name given to
 * a certificate with extra info tagged on the end. Since these
 * functions set how a certificate is trusted they should only
 * be used when the certificate comes from a reliable source
 * such as local storage.
 *
 */

X509 *d2i_X509_AUX(X509 **a, const unsigned char **pp, long length)
{
	const unsigned char *q;
	X509 *ret;
	/* Save start position */
	q = *pp;
	ret = d2i_X509(a, pp, length);
	/* If certificate unreadable then forget it */
	if(!ret) return NULL;
	/* update length */
	length -= *pp - q;
	if(!length) return ret;
	if(!d2i_X509_CERT_AUX(&ret->aux, pp, length)) goto err;
	return ret;
	err:
	X509_free(ret);
	return NULL;
}

int i2d_X509_AUX(X509 *a, unsigned char **pp)
{
	int length;
	length = i2d_X509(a, pp);
	if(a) length += i2d_X509_CERT_AUX(a->aux, pp);
	return length;
}