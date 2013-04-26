
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

/* PKCS#12 PBE algorithms now in static table */

void PKCS12_PBE_add(void)
{
}

int PKCS12_PBE_keyivgen(EVP_CIPHER_CTX *ctx, const char *pass, int passlen,
		ASN1_TYPE *param, const EVP_CIPHER *cipher, const EVP_MD *md, int en_de)
{
	PBEPARAM *pbe;
	int saltlen, iter, ret;
	unsigned char *salt;
	const unsigned char *pbuf;
	unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];

	/* Extract useful info from parameter */
	if (param == NULL || param->type != V_ASN1_SEQUENCE ||
	    param->value.sequence == NULL) {
		PKCS12err(PKCS12_F_PKCS12_PBE_KEYIVGEN,PKCS12_R_DECODE_ERROR);
		return 0;
	}

	pbuf = param->value.sequence->data;
	if (!(pbe = d2i_PBEPARAM(NULL, &pbuf, param->value.sequence->length))) {
		PKCS12err(PKCS12_F_PKCS12_PBE_KEYIVGEN,PKCS12_R_DECODE_ERROR);
		return 0;
	}

	if (!pbe->iter) iter = 1;
	else iter = ASN1_INTEGER_get (pbe->iter);
	salt = pbe->salt->data;
	saltlen = pbe->salt->length;
	if (!PKCS12_key_gen (pass, passlen, salt, saltlen, PKCS12_KEY_ID,
			     iter, EVP_CIPHER_key_length(cipher), key, md)) {
		PKCS12err(PKCS12_F_PKCS12_PBE_KEYIVGEN,PKCS12_R_KEY_GEN_ERROR);
		PBEPARAM_free(pbe);
		return 0;
	}
	if (!PKCS12_key_gen (pass, passlen, salt, saltlen, PKCS12_IV_ID,
				iter, EVP_CIPHER_iv_length(cipher), iv, md)) {
		PKCS12err(PKCS12_F_PKCS12_PBE_KEYIVGEN,PKCS12_R_IV_GEN_ERROR);
		PBEPARAM_free(pbe);
		return 0;
	}
	PBEPARAM_free(pbe);
	ret = EVP_CipherInit_ex(ctx, cipher, NULL, key, iv, en_de);
	OPENSSL_cleanse(key, EVP_MAX_KEY_LENGTH);
	OPENSSL_cleanse(iv, EVP_MAX_IV_LENGTH);
	return ret;
}