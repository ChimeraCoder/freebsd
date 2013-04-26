
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

#include "cryptlib.h"
#include <openssl/asn1t.h>
#include <openssl/pem.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/cms.h>
#include "cms_lcl.h"

DECLARE_ASN1_ITEM(CMS_DigestedData)

/* CMS DigestedData Utilities */

CMS_ContentInfo *cms_DigestedData_create(const EVP_MD *md)
	{
	CMS_ContentInfo *cms;
	CMS_DigestedData *dd;
	cms = CMS_ContentInfo_new();
	if (!cms)
		return NULL;

	dd = M_ASN1_new_of(CMS_DigestedData);

	if (!dd)
		goto err;

	cms->contentType = OBJ_nid2obj(NID_pkcs7_digest);
	cms->d.digestedData = dd;

	dd->version = 0;
	dd->encapContentInfo->eContentType = OBJ_nid2obj(NID_pkcs7_data);

	cms_DigestAlgorithm_set(dd->digestAlgorithm, md);

	return cms;

	err:

	if (cms)
		CMS_ContentInfo_free(cms);

	return NULL;
	}

BIO *cms_DigestedData_init_bio(CMS_ContentInfo *cms)
	{
	CMS_DigestedData *dd;
	dd = cms->d.digestedData;
	return cms_DigestAlgorithm_init_bio(dd->digestAlgorithm);
	}

int cms_DigestedData_do_final(CMS_ContentInfo *cms, BIO *chain, int verify)
	{
	EVP_MD_CTX mctx;
	unsigned char md[EVP_MAX_MD_SIZE];
	unsigned int mdlen;
	int r = 0;
	CMS_DigestedData *dd;
	EVP_MD_CTX_init(&mctx);

	dd = cms->d.digestedData;

	if (!cms_DigestAlgorithm_find_ctx(&mctx, chain, dd->digestAlgorithm))
		goto err;

	if (EVP_DigestFinal_ex(&mctx, md, &mdlen) <= 0)
		goto err;

	if (verify)
		{
		if (mdlen != (unsigned int)dd->digest->length)
			{
			CMSerr(CMS_F_CMS_DIGESTEDDATA_DO_FINAL,
				CMS_R_MESSAGEDIGEST_WRONG_LENGTH);
			goto err;
			}

		if (memcmp(md, dd->digest->data, mdlen))
			CMSerr(CMS_F_CMS_DIGESTEDDATA_DO_FINAL,
				CMS_R_VERIFICATION_FAILURE);
		else
			r = 1;
		}
	else
		{
		if (!ASN1_STRING_set(dd->digest, md, mdlen))
			goto err;
		r = 1;
		}

	err:
	EVP_MD_CTX_cleanup(&mctx);

	return r;

	}