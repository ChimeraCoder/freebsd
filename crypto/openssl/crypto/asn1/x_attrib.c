
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
#include <openssl/objects.h>
#include <openssl/asn1t.h>
#include <openssl/x509.h>

/* X509_ATTRIBUTE: this has the following form:
 *
 * typedef struct x509_attributes_st
 *	{
 *	ASN1_OBJECT *object;
 *	int single;
 *	union	{
 *		char		*ptr;
 * 		STACK_OF(ASN1_TYPE) *set;
 * 		ASN1_TYPE	*single;
 *		} value;
 *	} X509_ATTRIBUTE;
 *
 * this needs some extra thought because the CHOICE type is
 * merged with the main structure and because the value can
 * be anything at all we *must* try the SET OF first because
 * the ASN1_ANY type will swallow anything including the whole
 * SET OF structure.
 */

ASN1_CHOICE(X509_ATTRIBUTE_SET) = {
	ASN1_SET_OF(X509_ATTRIBUTE, value.set, ASN1_ANY),
	ASN1_SIMPLE(X509_ATTRIBUTE, value.single, ASN1_ANY)
} ASN1_CHOICE_END_selector(X509_ATTRIBUTE, X509_ATTRIBUTE_SET, single)

ASN1_SEQUENCE(X509_ATTRIBUTE) = {
	ASN1_SIMPLE(X509_ATTRIBUTE, object, ASN1_OBJECT),
	/* CHOICE type merged with parent */
	ASN1_EX_COMBINE(0, 0, X509_ATTRIBUTE_SET)
} ASN1_SEQUENCE_END(X509_ATTRIBUTE)

IMPLEMENT_ASN1_FUNCTIONS(X509_ATTRIBUTE)
IMPLEMENT_ASN1_DUP_FUNCTION(X509_ATTRIBUTE)

X509_ATTRIBUTE *X509_ATTRIBUTE_create(int nid, int atrtype, void *value)
	{
	X509_ATTRIBUTE *ret=NULL;
	ASN1_TYPE *val=NULL;

	if ((ret=X509_ATTRIBUTE_new()) == NULL)
		return(NULL);
	ret->object=OBJ_nid2obj(nid);
	ret->single=0;
	if ((ret->value.set=sk_ASN1_TYPE_new_null()) == NULL) goto err;
	if ((val=ASN1_TYPE_new()) == NULL) goto err;
	if (!sk_ASN1_TYPE_push(ret->value.set,val)) goto err;

	ASN1_TYPE_set(val,atrtype,value);
	return(ret);
err:
	if (ret != NULL) X509_ATTRIBUTE_free(ret);
	if (val != NULL) ASN1_TYPE_free(val);
	return(NULL);
	}