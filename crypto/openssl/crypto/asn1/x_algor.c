
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

#include <stddef.h>
#include <openssl/x509.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>

ASN1_SEQUENCE(X509_ALGOR) = {
	ASN1_SIMPLE(X509_ALGOR, algorithm, ASN1_OBJECT),
	ASN1_OPT(X509_ALGOR, parameter, ASN1_ANY)
} ASN1_SEQUENCE_END(X509_ALGOR)

ASN1_ITEM_TEMPLATE(X509_ALGORS) = 
	ASN1_EX_TEMPLATE_TYPE(ASN1_TFLG_SEQUENCE_OF, 0, algorithms, X509_ALGOR)
ASN1_ITEM_TEMPLATE_END(X509_ALGORS)

IMPLEMENT_ASN1_FUNCTIONS(X509_ALGOR)
IMPLEMENT_ASN1_ENCODE_FUNCTIONS_fname(X509_ALGORS, X509_ALGORS, X509_ALGORS)
IMPLEMENT_ASN1_DUP_FUNCTION(X509_ALGOR)

IMPLEMENT_STACK_OF(X509_ALGOR)
IMPLEMENT_ASN1_SET_OF(X509_ALGOR)

int X509_ALGOR_set0(X509_ALGOR *alg, ASN1_OBJECT *aobj, int ptype, void *pval)
	{
	if (!alg)
		return 0;
	if (ptype != V_ASN1_UNDEF)
		{
		if (alg->parameter == NULL)
			alg->parameter = ASN1_TYPE_new();
		if (alg->parameter == NULL)
			return 0;
		}
	if (alg)
		{
		if (alg->algorithm)
			ASN1_OBJECT_free(alg->algorithm);
		alg->algorithm = aobj;
		}
	if (ptype == 0)
		return 1;	
	if (ptype == V_ASN1_UNDEF)
		{
		if (alg->parameter)
			{
			ASN1_TYPE_free(alg->parameter);
			alg->parameter = NULL;
			}
		}
	else
		ASN1_TYPE_set(alg->parameter, ptype, pval);
	return 1;
	}

void X509_ALGOR_get0(ASN1_OBJECT **paobj, int *pptype, void **ppval,
						X509_ALGOR *algor)
	{
	if (paobj)
		*paobj = algor->algorithm;
	if (pptype)
		{
		if (algor->parameter == NULL)
			{
			*pptype = V_ASN1_UNDEF;
			return;
			}
		else
			*pptype = algor->parameter->type;
		if (ppval)
			*ppval = algor->parameter->value.ptr;
		}
	}

/* Set up an X509_ALGOR DigestAlgorithmIdentifier from an EVP_MD */

void X509_ALGOR_set_md(X509_ALGOR *alg, const EVP_MD *md)
	{
	int param_type;

	if (md->flags & EVP_MD_FLAG_DIGALGID_ABSENT)
		param_type = V_ASN1_UNDEF;
	else
		param_type = V_ASN1_NULL;

	X509_ALGOR_set0(alg, OBJ_nid2obj(EVP_MD_type(md)), param_type, NULL);

	}