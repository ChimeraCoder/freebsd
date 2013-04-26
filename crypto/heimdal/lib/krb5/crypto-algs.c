
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

#include "krb5_locl.h"

#ifndef HEIMDAL_SMALLER
#define DES3_OLD_ENCTYPE 1
#endif

struct _krb5_checksum_type *_krb5_checksum_types[] = {
    &_krb5_checksum_none,
#ifdef HEIM_WEAK_CRYPTO
    &_krb5_checksum_crc32,
    &_krb5_checksum_rsa_md4,
    &_krb5_checksum_rsa_md4_des,
    &_krb5_checksum_rsa_md5_des,
#endif
#ifdef DES3_OLD_ENCTYPE
    &_krb5_checksum_rsa_md5_des3,
#endif
    &_krb5_checksum_rsa_md5,
    &_krb5_checksum_sha1,
    &_krb5_checksum_hmac_sha1_des3,
    &_krb5_checksum_hmac_sha1_aes128,
    &_krb5_checksum_hmac_sha1_aes256,
    &_krb5_checksum_hmac_md5
};

int _krb5_num_checksums
	= sizeof(_krb5_checksum_types) / sizeof(_krb5_checksum_types[0]);

/*
 * these should currently be in reverse preference order.
 * (only relevant for !F_PSEUDO) */

struct _krb5_encryption_type *_krb5_etypes[] = {
    &_krb5_enctype_aes256_cts_hmac_sha1,
    &_krb5_enctype_aes128_cts_hmac_sha1,
    &_krb5_enctype_des3_cbc_sha1,
    &_krb5_enctype_des3_cbc_none, /* used by the gss-api mech */
    &_krb5_enctype_arcfour_hmac_md5,
#ifdef DES3_OLD_ENCTYPE
    &_krb5_enctype_des3_cbc_md5,
    &_krb5_enctype_old_des3_cbc_sha1,
#endif
#ifdef HEIM_WEAK_CRYPTO
    &_krb5_enctype_des_cbc_md5,
    &_krb5_enctype_des_cbc_md4,
    &_krb5_enctype_des_cbc_crc,
    &_krb5_enctype_des_cbc_none,
    &_krb5_enctype_des_cfb64_none,
    &_krb5_enctype_des_pcbc_none,
#endif
    &_krb5_enctype_null
};

int _krb5_num_etypes = sizeof(_krb5_etypes) / sizeof(_krb5_etypes[0]);