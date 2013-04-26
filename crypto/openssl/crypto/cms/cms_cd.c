
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
#include <openssl/bio.h>
#ifndef OPENSSL_NO_COMP
#include <openssl/comp.h>
#endif
#include "cms_lcl.h"

DECLARE_ASN1_ITEM(CMS_CompressedData)

#ifdef ZLIB

/* CMS CompressedData Utilities */

CMS_ContentInfo *cms_CompressedData_create(int comp_nid)
	{
	CMS_ContentInfo *cms;
	CMS_CompressedData *cd;
	/* Will need something cleverer if there is ever more than one
	 * compression algorithm or parameters have some meaning...
	 */
	if (comp_nid != NID_zlib_compression)
		{
		CMSerr(CMS_F_CMS_COMPRESSEDDATA_CREATE,
				CMS_R_UNSUPPORTED_COMPRESSION_ALGORITHM);
		return NULL;
		}
	cms = CMS_ContentInfo_new();
	if (!cms)
		return NULL;

	cd = M_ASN1_new_of(CMS_CompressedData);

	if (!cd)
		goto err;

	cms->contentType = OBJ_nid2obj(NID_id_smime_ct_compressedData);
	cms->d.compressedData = cd;

	cd->version = 0;

	X509_ALGOR_set0(cd->compressionAlgorithm,
			OBJ_nid2obj(NID_zlib_compression),
			V_ASN1_UNDEF, NULL);

	cd->encapContentInfo->eContentType = OBJ_nid2obj(NID_pkcs7_data);

	return cms;

	err:

	if (cms)
		CMS_ContentInfo_free(cms);

	return NULL;
	}

BIO *cms_CompressedData_init_bio(CMS_ContentInfo *cms)
	{
	CMS_CompressedData *cd;
	ASN1_OBJECT *compoid;
	if (OBJ_obj2nid(cms->contentType) != NID_id_smime_ct_compressedData)
		{
		CMSerr(CMS_F_CMS_COMPRESSEDDATA_INIT_BIO,
				CMS_R_CONTENT_TYPE_NOT_COMPRESSED_DATA);
		return NULL;
		}
	cd = cms->d.compressedData;
	X509_ALGOR_get0(&compoid, NULL, NULL, cd->compressionAlgorithm);
	if (OBJ_obj2nid(compoid) != NID_zlib_compression)
		{
		CMSerr(CMS_F_CMS_COMPRESSEDDATA_INIT_BIO,
				CMS_R_UNSUPPORTED_COMPRESSION_ALGORITHM);
		return NULL;
		}
	return BIO_new(BIO_f_zlib());
	}

#endif