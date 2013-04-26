
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

/* X509_CERT_AUX routines. These are used to encode additional
 * user modifiable data about a certificate. This data is
 * appended to the X509 encoding when the *_X509_AUX routines
 * are used. This means that the "traditional" X509 routines
 * will simply ignore the extra data. 
 */

static X509_CERT_AUX *aux_get(X509 *x);

ASN1_SEQUENCE(X509_CERT_AUX) = {
	ASN1_SEQUENCE_OF_OPT(X509_CERT_AUX, trust, ASN1_OBJECT),
	ASN1_IMP_SEQUENCE_OF_OPT(X509_CERT_AUX, reject, ASN1_OBJECT, 0),
	ASN1_OPT(X509_CERT_AUX, alias, ASN1_UTF8STRING),
	ASN1_OPT(X509_CERT_AUX, keyid, ASN1_OCTET_STRING),
	ASN1_IMP_SEQUENCE_OF_OPT(X509_CERT_AUX, other, X509_ALGOR, 1)
} ASN1_SEQUENCE_END(X509_CERT_AUX)

IMPLEMENT_ASN1_FUNCTIONS(X509_CERT_AUX)

static X509_CERT_AUX *aux_get(X509 *x)
{
	if(!x) return NULL;
	if(!x->aux && !(x->aux = X509_CERT_AUX_new())) return NULL;
	return x->aux;
}

int X509_alias_set1(X509 *x, unsigned char *name, int len)
{
	X509_CERT_AUX *aux;
	if (!name)
		{
		if (!x || !x->aux || !x->aux->alias)
			return 1;
		ASN1_UTF8STRING_free(x->aux->alias);
		x->aux->alias = NULL;
		return 1;
		}
	if(!(aux = aux_get(x))) return 0;
	if(!aux->alias && !(aux->alias = ASN1_UTF8STRING_new())) return 0;
	return ASN1_STRING_set(aux->alias, name, len);
}

int X509_keyid_set1(X509 *x, unsigned char *id, int len)
{
	X509_CERT_AUX *aux;
	if (!id)
		{
		if (!x || !x->aux || !x->aux->keyid)
			return 1;
		ASN1_OCTET_STRING_free(x->aux->keyid);
		x->aux->keyid = NULL;
		return 1;
		}
	if(!(aux = aux_get(x))) return 0;
	if(!aux->keyid && !(aux->keyid = ASN1_OCTET_STRING_new())) return 0;
	return ASN1_STRING_set(aux->keyid, id, len);
}

unsigned char *X509_alias_get0(X509 *x, int *len)
{
	if(!x->aux || !x->aux->alias) return NULL;
	if(len) *len = x->aux->alias->length;
	return x->aux->alias->data;
}

unsigned char *X509_keyid_get0(X509 *x, int *len)
{
	if(!x->aux || !x->aux->keyid) return NULL;
	if(len) *len = x->aux->keyid->length;
	return x->aux->keyid->data;
}

int X509_add1_trust_object(X509 *x, ASN1_OBJECT *obj)
{
	X509_CERT_AUX *aux;
	ASN1_OBJECT *objtmp;
	if(!(objtmp = OBJ_dup(obj))) return 0;
	if(!(aux = aux_get(x))) return 0;
	if(!aux->trust
		&& !(aux->trust = sk_ASN1_OBJECT_new_null())) return 0;
	return sk_ASN1_OBJECT_push(aux->trust, objtmp);
}

int X509_add1_reject_object(X509 *x, ASN1_OBJECT *obj)
{
	X509_CERT_AUX *aux;
	ASN1_OBJECT *objtmp;
	if(!(objtmp = OBJ_dup(obj))) return 0;
	if(!(aux = aux_get(x))) return 0;
	if(!aux->reject
		&& !(aux->reject = sk_ASN1_OBJECT_new_null())) return 0;
	return sk_ASN1_OBJECT_push(aux->reject, objtmp);
}

void X509_trust_clear(X509 *x)
{
	if(x->aux && x->aux->trust) {
		sk_ASN1_OBJECT_pop_free(x->aux->trust, ASN1_OBJECT_free);
		x->aux->trust = NULL;
	}
}

void X509_reject_clear(X509 *x)
{
	if(x->aux && x->aux->reject) {
		sk_ASN1_OBJECT_pop_free(x->aux->reject, ASN1_OBJECT_free);
		x->aux->reject = NULL;
	}
}

ASN1_SEQUENCE(X509_CERT_PAIR) = {
	ASN1_EXP_OPT(X509_CERT_PAIR, forward, X509, 0),
	ASN1_EXP_OPT(X509_CERT_PAIR, reverse, X509, 1)
} ASN1_SEQUENCE_END(X509_CERT_PAIR)

IMPLEMENT_ASN1_FUNCTIONS(X509_CERT_PAIR)