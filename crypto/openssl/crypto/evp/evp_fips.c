
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


#include <openssl/evp.h>

#ifdef OPENSSL_FIPS
#include <openssl/fips.h>

const EVP_CIPHER *EVP_aes_128_cbc(void)  { return FIPS_evp_aes_128_cbc(); }
const EVP_CIPHER *EVP_aes_128_ccm(void)  { return FIPS_evp_aes_128_ccm(); }
const EVP_CIPHER *EVP_aes_128_cfb1(void)  { return FIPS_evp_aes_128_cfb1(); }
const EVP_CIPHER *EVP_aes_128_cfb128(void)  { return FIPS_evp_aes_128_cfb128(); }
const EVP_CIPHER *EVP_aes_128_cfb8(void)  { return FIPS_evp_aes_128_cfb8(); }
const EVP_CIPHER *EVP_aes_128_ctr(void)  { return FIPS_evp_aes_128_ctr(); }
const EVP_CIPHER *EVP_aes_128_ecb(void)  { return FIPS_evp_aes_128_ecb(); }
const EVP_CIPHER *EVP_aes_128_gcm(void)  { return FIPS_evp_aes_128_gcm(); }
const EVP_CIPHER *EVP_aes_128_ofb(void)  { return FIPS_evp_aes_128_ofb(); }
const EVP_CIPHER *EVP_aes_128_xts(void)  { return FIPS_evp_aes_128_xts(); }
const EVP_CIPHER *EVP_aes_192_cbc(void)  { return FIPS_evp_aes_192_cbc(); }
const EVP_CIPHER *EVP_aes_192_ccm(void)  { return FIPS_evp_aes_192_ccm(); }
const EVP_CIPHER *EVP_aes_192_cfb1(void)  { return FIPS_evp_aes_192_cfb1(); }
const EVP_CIPHER *EVP_aes_192_cfb128(void)  { return FIPS_evp_aes_192_cfb128(); }
const EVP_CIPHER *EVP_aes_192_cfb8(void)  { return FIPS_evp_aes_192_cfb8(); }
const EVP_CIPHER *EVP_aes_192_ctr(void)  { return FIPS_evp_aes_192_ctr(); }
const EVP_CIPHER *EVP_aes_192_ecb(void)  { return FIPS_evp_aes_192_ecb(); }
const EVP_CIPHER *EVP_aes_192_gcm(void)  { return FIPS_evp_aes_192_gcm(); }
const EVP_CIPHER *EVP_aes_192_ofb(void)  { return FIPS_evp_aes_192_ofb(); }
const EVP_CIPHER *EVP_aes_256_cbc(void)  { return FIPS_evp_aes_256_cbc(); }
const EVP_CIPHER *EVP_aes_256_ccm(void)  { return FIPS_evp_aes_256_ccm(); }
const EVP_CIPHER *EVP_aes_256_cfb1(void)  { return FIPS_evp_aes_256_cfb1(); }
const EVP_CIPHER *EVP_aes_256_cfb128(void)  { return FIPS_evp_aes_256_cfb128(); }
const EVP_CIPHER *EVP_aes_256_cfb8(void)  { return FIPS_evp_aes_256_cfb8(); }
const EVP_CIPHER *EVP_aes_256_ctr(void)  { return FIPS_evp_aes_256_ctr(); }
const EVP_CIPHER *EVP_aes_256_ecb(void)  { return FIPS_evp_aes_256_ecb(); }
const EVP_CIPHER *EVP_aes_256_gcm(void)  { return FIPS_evp_aes_256_gcm(); }
const EVP_CIPHER *EVP_aes_256_ofb(void)  { return FIPS_evp_aes_256_ofb(); }
const EVP_CIPHER *EVP_aes_256_xts(void)  { return FIPS_evp_aes_256_xts(); }
const EVP_CIPHER *EVP_des_ede(void)  { return FIPS_evp_des_ede(); }
const EVP_CIPHER *EVP_des_ede3(void)  { return FIPS_evp_des_ede3(); }
const EVP_CIPHER *EVP_des_ede3_cbc(void)  { return FIPS_evp_des_ede3_cbc(); }
const EVP_CIPHER *EVP_des_ede3_cfb1(void)  { return FIPS_evp_des_ede3_cfb1(); }
const EVP_CIPHER *EVP_des_ede3_cfb64(void)  { return FIPS_evp_des_ede3_cfb64(); }
const EVP_CIPHER *EVP_des_ede3_cfb8(void)  { return FIPS_evp_des_ede3_cfb8(); }
const EVP_CIPHER *EVP_des_ede3_ecb(void)  { return FIPS_evp_des_ede3_ecb(); }
const EVP_CIPHER *EVP_des_ede3_ofb(void)  { return FIPS_evp_des_ede3_ofb(); }
const EVP_CIPHER *EVP_des_ede_cbc(void)  { return FIPS_evp_des_ede_cbc(); }
const EVP_CIPHER *EVP_des_ede_cfb64(void)  { return FIPS_evp_des_ede_cfb64(); }
const EVP_CIPHER *EVP_des_ede_ecb(void)  { return FIPS_evp_des_ede_ecb(); }
const EVP_CIPHER *EVP_des_ede_ofb(void)  { return FIPS_evp_des_ede_ofb(); }
const EVP_CIPHER *EVP_enc_null(void)  { return FIPS_evp_enc_null(); }

const EVP_MD *EVP_sha1(void)  { return FIPS_evp_sha1(); }
const EVP_MD *EVP_sha224(void)  { return FIPS_evp_sha224(); }
const EVP_MD *EVP_sha256(void)  { return FIPS_evp_sha256(); }
const EVP_MD *EVP_sha384(void)  { return FIPS_evp_sha384(); }
const EVP_MD *EVP_sha512(void)  { return FIPS_evp_sha512(); }

const EVP_MD *EVP_dss(void)  { return FIPS_evp_dss(); }
const EVP_MD *EVP_dss1(void)  { return FIPS_evp_dss1(); }
const EVP_MD *EVP_ecdsa(void)  { return FIPS_evp_ecdsa(); }

#endif