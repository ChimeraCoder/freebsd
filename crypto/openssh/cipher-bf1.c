
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

#include "includes.h"

#include <sys/types.h>

#include <openssl/evp.h>

#include <stdarg.h>
#include <string.h>

#include "xmalloc.h"
#include "log.h"

#include "openbsd-compat/openssl-compat.h"

/*
 * SSH1 uses a variation on Blowfish, all bytes must be swapped before
 * and after encryption/decryption. Thus the swap_bytes stuff (yuk).
 */

const EVP_CIPHER * evp_ssh1_bf(void);

static void
swap_bytes(const u_char *src, u_char *dst, int n)
{
	u_char c[4];

	/* Process 4 bytes every lap. */
	for (n = n / 4; n > 0; n--) {
		c[3] = *src++;
		c[2] = *src++;
		c[1] = *src++;
		c[0] = *src++;

		*dst++ = c[0];
		*dst++ = c[1];
		*dst++ = c[2];
		*dst++ = c[3];
	}
}

#ifdef SSH_OLD_EVP
static void bf_ssh1_init (EVP_CIPHER_CTX * ctx, const unsigned char *key,
			  const unsigned char *iv, int enc)
{
	if (iv != NULL)
		memcpy (&(ctx->oiv[0]), iv, 8);
	memcpy (&(ctx->iv[0]), &(ctx->oiv[0]), 8);
	if (key != NULL)
		BF_set_key (&(ctx->c.bf_ks), EVP_CIPHER_CTX_key_length (ctx),
			    key);
}
#endif

static int (*orig_bf)(EVP_CIPHER_CTX *, u_char *,
    const u_char *, LIBCRYPTO_EVP_INL_TYPE) = NULL;

static int
bf_ssh1_cipher(EVP_CIPHER_CTX *ctx, u_char *out, const u_char *in,
    LIBCRYPTO_EVP_INL_TYPE len)
{
	int ret;

	swap_bytes(in, out, len);
	ret = (*orig_bf)(ctx, out, out, len);
	swap_bytes(out, out, len);
	return (ret);
}

const EVP_CIPHER *
evp_ssh1_bf(void)
{
	static EVP_CIPHER ssh1_bf;

	memcpy(&ssh1_bf, EVP_bf_cbc(), sizeof(EVP_CIPHER));
	orig_bf = ssh1_bf.do_cipher;
	ssh1_bf.nid = NID_undef;
#ifdef SSH_OLD_EVP
	ssh1_bf.init = bf_ssh1_init;
#endif
	ssh1_bf.do_cipher = bf_ssh1_cipher;
	ssh1_bf.key_len = 32;
	return (&ssh1_bf);
}