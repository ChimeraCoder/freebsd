
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
#include <openssl/pkcs12.h>

/* Pack an object into an OCTET STRING and turn into a safebag */

PKCS12_SAFEBAG *PKCS12_item_pack_safebag(void *obj, const ASN1_ITEM *it, int nid1,
	     int nid2)
{
	PKCS12_BAGS *bag;
	PKCS12_SAFEBAG *safebag;
	if (!(bag = PKCS12_BAGS_new())) {
		PKCS12err(PKCS12_F_PKCS12_ITEM_PACK_SAFEBAG, ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	bag->type = OBJ_nid2obj(nid1);
	if (!ASN1_item_pack(obj, it, &bag->value.octet)) {
		PKCS12err(PKCS12_F_PKCS12_ITEM_PACK_SAFEBAG, ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	if (!(safebag = PKCS12_SAFEBAG_new())) {
		PKCS12err(PKCS12_F_PKCS12_ITEM_PACK_SAFEBAG, ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	safebag->value.bag = bag;
	safebag->type = OBJ_nid2obj(nid2);
	return safebag;
}

/* Turn PKCS8 object into a keybag */

PKCS12_SAFEBAG *PKCS12_MAKE_KEYBAG(PKCS8_PRIV_KEY_INFO *p8)
{
	PKCS12_SAFEBAG *bag;
	if (!(bag = PKCS12_SAFEBAG_new())) {
		PKCS12err(PKCS12_F_PKCS12_MAKE_KEYBAG,ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	bag->type = OBJ_nid2obj(NID_keyBag);
	bag->value.keybag = p8;
	return bag;
}

/* Turn PKCS8 object into a shrouded keybag */

PKCS12_SAFEBAG *PKCS12_MAKE_SHKEYBAG(int pbe_nid, const char *pass,
	     int passlen, unsigned char *salt, int saltlen, int iter,
	     PKCS8_PRIV_KEY_INFO *p8)
{
	PKCS12_SAFEBAG *bag;
	const EVP_CIPHER *pbe_ciph;

	/* Set up the safe bag */
	if (!(bag = PKCS12_SAFEBAG_new())) {
		PKCS12err(PKCS12_F_PKCS12_MAKE_SHKEYBAG, ERR_R_MALLOC_FAILURE);
		return NULL;
	}

	bag->type = OBJ_nid2obj(NID_pkcs8ShroudedKeyBag);

	pbe_ciph = EVP_get_cipherbynid(pbe_nid);

	if (pbe_ciph)
		pbe_nid = -1;

	if (!(bag->value.shkeybag = 
	  PKCS8_encrypt(pbe_nid, pbe_ciph, pass, passlen, salt, saltlen, iter,
									 p8))) {
		PKCS12err(PKCS12_F_PKCS12_MAKE_SHKEYBAG, ERR_R_MALLOC_FAILURE);
		return NULL;
	}

	return bag;
}

/* Turn a stack of SAFEBAGS into a PKCS#7 data Contentinfo */
PKCS7 *PKCS12_pack_p7data(STACK_OF(PKCS12_SAFEBAG) *sk)
{
	PKCS7 *p7;
	if (!(p7 = PKCS7_new())) {
		PKCS12err(PKCS12_F_PKCS12_PACK_P7DATA, ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	p7->type = OBJ_nid2obj(NID_pkcs7_data);
	if (!(p7->d.data = M_ASN1_OCTET_STRING_new())) {
		PKCS12err(PKCS12_F_PKCS12_PACK_P7DATA, ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	
	if (!ASN1_item_pack(sk, ASN1_ITEM_rptr(PKCS12_SAFEBAGS), &p7->d.data)) {
		PKCS12err(PKCS12_F_PKCS12_PACK_P7DATA, PKCS12_R_CANT_PACK_STRUCTURE);
		return NULL;
	}
	return p7;
}

/* Unpack SAFEBAGS from PKCS#7 data ContentInfo */
STACK_OF(PKCS12_SAFEBAG) *PKCS12_unpack_p7data(PKCS7 *p7)
{
	if(!PKCS7_type_is_data(p7))
		{
		PKCS12err(PKCS12_F_PKCS12_UNPACK_P7DATA,PKCS12_R_CONTENT_TYPE_NOT_DATA);
		return NULL;
		}
	return ASN1_item_unpack(p7->d.data, ASN1_ITEM_rptr(PKCS12_SAFEBAGS));
}

/* Turn a stack of SAFEBAGS into a PKCS#7 encrypted data ContentInfo */

PKCS7 *PKCS12_pack_p7encdata(int pbe_nid, const char *pass, int passlen,
			      unsigned char *salt, int saltlen, int iter,
			      STACK_OF(PKCS12_SAFEBAG) *bags)
{
	PKCS7 *p7;
	X509_ALGOR *pbe;
	const EVP_CIPHER *pbe_ciph;
	if (!(p7 = PKCS7_new())) {
		PKCS12err(PKCS12_F_PKCS12_PACK_P7ENCDATA, ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	if(!PKCS7_set_type(p7, NID_pkcs7_encrypted)) {
		PKCS12err(PKCS12_F_PKCS12_PACK_P7ENCDATA,
				PKCS12_R_ERROR_SETTING_ENCRYPTED_DATA_TYPE);
		return NULL;
	}

	pbe_ciph = EVP_get_cipherbynid(pbe_nid);

	if (pbe_ciph)
		pbe = PKCS5_pbe2_set(pbe_ciph, iter, salt, saltlen);
	else
		pbe = PKCS5_pbe_set(pbe_nid, iter, salt, saltlen);

	if (!pbe) {
		PKCS12err(PKCS12_F_PKCS12_PACK_P7ENCDATA, ERR_R_MALLOC_FAILURE);
		return NULL;
	}
	X509_ALGOR_free(p7->d.encrypted->enc_data->algorithm);
	p7->d.encrypted->enc_data->algorithm = pbe;
	M_ASN1_OCTET_STRING_free(p7->d.encrypted->enc_data->enc_data);
	if (!(p7->d.encrypted->enc_data->enc_data =
	PKCS12_item_i2d_encrypt(pbe, ASN1_ITEM_rptr(PKCS12_SAFEBAGS), pass, passlen,
				 bags, 1))) {
		PKCS12err(PKCS12_F_PKCS12_PACK_P7ENCDATA, PKCS12_R_ENCRYPT_ERROR);
		return NULL;
	}

	return p7;
}

STACK_OF(PKCS12_SAFEBAG) *PKCS12_unpack_p7encdata(PKCS7 *p7, const char *pass, int passlen)
{
	if(!PKCS7_type_is_encrypted(p7)) return NULL;
	return PKCS12_item_decrypt_d2i(p7->d.encrypted->enc_data->algorithm,
			           ASN1_ITEM_rptr(PKCS12_SAFEBAGS),
				   pass, passlen,
			           p7->d.encrypted->enc_data->enc_data, 1);
}

PKCS8_PRIV_KEY_INFO *PKCS12_decrypt_skey(PKCS12_SAFEBAG *bag, const char *pass,
								int passlen)
{
	return PKCS8_decrypt(bag->value.shkeybag, pass, passlen);
}

int PKCS12_pack_authsafes(PKCS12 *p12, STACK_OF(PKCS7) *safes) 
{
	if(ASN1_item_pack(safes, ASN1_ITEM_rptr(PKCS12_AUTHSAFES),
		&p12->authsafes->d.data)) 
			return 1;
	return 0;
}

STACK_OF(PKCS7) *PKCS12_unpack_authsafes(PKCS12 *p12)
{
	if (!PKCS7_type_is_data(p12->authsafes))
		{
		PKCS12err(PKCS12_F_PKCS12_UNPACK_AUTHSAFES,PKCS12_R_CONTENT_TYPE_NOT_DATA);
		return NULL;
		}
	return ASN1_item_unpack(p12->authsafes->d.data, ASN1_ITEM_rptr(PKCS12_AUTHSAFES));
}