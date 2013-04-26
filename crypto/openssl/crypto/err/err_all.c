
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
#include <openssl/asn1.h>
#include <openssl/bn.h>
#ifndef OPENSSL_NO_EC
#include <openssl/ec.h>
#endif
#include <openssl/buffer.h>
#include <openssl/bio.h>
#ifndef OPENSSL_NO_COMP
#include <openssl/comp.h>
#endif
#ifndef OPENSSL_NO_RSA
#include <openssl/rsa.h>
#endif
#ifndef OPENSSL_NO_DH
#include <openssl/dh.h>
#endif
#ifndef OPENSSL_NO_DSA
#include <openssl/dsa.h>
#endif
#ifndef OPENSSL_NO_ECDSA
#include <openssl/ecdsa.h>
#endif
#ifndef OPENSSL_NO_ECDH
#include <openssl/ecdh.h>
#endif
#include <openssl/evp.h>
#include <openssl/objects.h>
#include <openssl/pem2.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/conf.h>
#include <openssl/pkcs12.h>
#include <openssl/rand.h>
#include <openssl/dso.h>
#ifndef OPENSSL_NO_ENGINE
#include <openssl/engine.h>
#endif
#include <openssl/ui.h>
#include <openssl/ocsp.h>
#include <openssl/err.h>
#ifdef OPENSSL_FIPS
#include <openssl/fips.h>
#endif
#include <openssl/ts.h>
#ifndef OPENSSL_NO_CMS
#include <openssl/cms.h>
#endif
#ifndef OPENSSL_NO_JPAKE
#include <openssl/jpake.h>
#endif

void ERR_load_crypto_strings(void)
	{
#ifndef OPENSSL_NO_ERR
	ERR_load_ERR_strings(); /* include error strings for SYSerr */
	ERR_load_BN_strings();
#ifndef OPENSSL_NO_RSA
	ERR_load_RSA_strings();
#endif
#ifndef OPENSSL_NO_DH
	ERR_load_DH_strings();
#endif
	ERR_load_EVP_strings();
	ERR_load_BUF_strings();
	ERR_load_OBJ_strings();
	ERR_load_PEM_strings();
#ifndef OPENSSL_NO_DSA
	ERR_load_DSA_strings();
#endif
	ERR_load_X509_strings();
	ERR_load_ASN1_strings();
	ERR_load_CONF_strings();
	ERR_load_CRYPTO_strings();
#ifndef OPENSSL_NO_COMP
	ERR_load_COMP_strings();
#endif
#ifndef OPENSSL_NO_EC
	ERR_load_EC_strings();
#endif
#ifndef OPENSSL_NO_ECDSA
	ERR_load_ECDSA_strings();
#endif
#ifndef OPENSSL_NO_ECDH
	ERR_load_ECDH_strings();
#endif
	/* skip ERR_load_SSL_strings() because it is not in this library */
	ERR_load_BIO_strings();
	ERR_load_PKCS7_strings();	
	ERR_load_X509V3_strings();
	ERR_load_PKCS12_strings();
	ERR_load_RAND_strings();
	ERR_load_DSO_strings();
	ERR_load_TS_strings();
#ifndef OPENSSL_NO_ENGINE
	ERR_load_ENGINE_strings();
#endif
	ERR_load_OCSP_strings();
	ERR_load_UI_strings();
#ifdef OPENSSL_FIPS
	ERR_load_FIPS_strings();
#endif
#ifndef OPENSSL_NO_CMS
	ERR_load_CMS_strings();
#endif
#ifndef OPENSSL_NO_JPAKE
	ERR_load_JPAKE_strings();
#endif
#endif
	}