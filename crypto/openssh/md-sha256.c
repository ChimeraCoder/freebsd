
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

/* EVP wrapper for SHA256 */

#include "includes.h"

#include <sys/types.h>
#include <openssl/opensslv.h>

#if !defined(HAVE_EVP_SHA256) && (OPENSSL_VERSION_NUMBER >= 0x00907000L)

#include <string.h>
#include <openssl/evp.h>
#ifdef HAVE_SHA256_UPDATE
# ifdef HAVE_SHA2_H
#  include <sha2.h>
# elif defined(HAVE_CRYPTO_SHA2_H)
#  include <crypto/sha2.h>
# endif
#endif

const EVP_MD *evp_ssh_sha256(void);

static int
ssh_sha256_init(EVP_MD_CTX *ctxt)
{
	SHA256_Init(ctxt->md_data);
	return (1);
}

static int
ssh_sha256_update(EVP_MD_CTX *ctxt, const void *data, unsigned long len)
{
	SHA256_Update(ctxt->md_data, data, len);
	return (1);
}

static int
ssh_sha256_final(EVP_MD_CTX *ctxt, unsigned char *digest)
{
	SHA256_Final(digest, ctxt->md_data);
	return (1);
}

static int
ssh_sha256_cleanup(EVP_MD_CTX *ctxt)
{
	memset(ctxt->md_data, 0, sizeof(SHA256_CTX));
	return (1);
}

const EVP_MD *
evp_ssh_sha256(void)
{
	static EVP_MD ssh_sha256;

	memset(&ssh_sha256, 0, sizeof(ssh_sha256));
	ssh_sha256.type = NID_undef;
	ssh_sha256.md_size = SHA256_DIGEST_LENGTH;
	ssh_sha256.init = ssh_sha256_init;
	ssh_sha256.update = ssh_sha256_update;
	ssh_sha256.final = ssh_sha256_final;
	ssh_sha256.cleanup = ssh_sha256_cleanup;
	ssh_sha256.block_size = SHA256_BLOCK_LENGTH;
	ssh_sha256.ctx_size = sizeof(SHA256_CTX);

	return (&ssh_sha256);
}

#endif /* !defined(HAVE_EVP_SHA256) && (OPENSSL_VERSION_NUMBER >= 0x00907000L) */