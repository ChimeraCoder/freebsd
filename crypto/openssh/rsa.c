
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

#include <stdarg.h>
#include <string.h>

#include "xmalloc.h"
#include "rsa.h"
#include "log.h"

void
rsa_public_encrypt(BIGNUM *out, BIGNUM *in, RSA *key)
{
	u_char *inbuf, *outbuf;
	int len, ilen, olen;

	if (BN_num_bits(key->e) < 2 || !BN_is_odd(key->e))
		fatal("rsa_public_encrypt() exponent too small or not odd");

	olen = BN_num_bytes(key->n);
	outbuf = xmalloc(olen);

	ilen = BN_num_bytes(in);
	inbuf = xmalloc(ilen);
	BN_bn2bin(in, inbuf);

	if ((len = RSA_public_encrypt(ilen, inbuf, outbuf, key,
	    RSA_PKCS1_PADDING)) <= 0)
		fatal("rsa_public_encrypt() failed");

	if (BN_bin2bn(outbuf, len, out) == NULL)
		fatal("rsa_public_encrypt: BN_bin2bn failed");

	memset(outbuf, 0, olen);
	memset(inbuf, 0, ilen);
	xfree(outbuf);
	xfree(inbuf);
}

int
rsa_private_decrypt(BIGNUM *out, BIGNUM *in, RSA *key)
{
	u_char *inbuf, *outbuf;
	int len, ilen, olen;

	olen = BN_num_bytes(key->n);
	outbuf = xmalloc(olen);

	ilen = BN_num_bytes(in);
	inbuf = xmalloc(ilen);
	BN_bn2bin(in, inbuf);

	if ((len = RSA_private_decrypt(ilen, inbuf, outbuf, key,
	    RSA_PKCS1_PADDING)) <= 0) {
		error("rsa_private_decrypt() failed");
	} else {
		if (BN_bin2bn(outbuf, len, out) == NULL)
			fatal("rsa_private_decrypt: BN_bin2bn failed");
	}
	memset(outbuf, 0, olen);
	memset(inbuf, 0, ilen);
	xfree(outbuf);
	xfree(inbuf);
	return len;
}

/* calculate p-1 and q-1 */
void
rsa_generate_additional_parameters(RSA *rsa)
{
	BIGNUM *aux;
	BN_CTX *ctx;

	if ((aux = BN_new()) == NULL)
		fatal("rsa_generate_additional_parameters: BN_new failed");
	if ((ctx = BN_CTX_new()) == NULL)
		fatal("rsa_generate_additional_parameters: BN_CTX_new failed");

	if ((BN_sub(aux, rsa->q, BN_value_one()) == 0) ||
	    (BN_mod(rsa->dmq1, rsa->d, aux, ctx) == 0) ||
	    (BN_sub(aux, rsa->p, BN_value_one()) == 0) ||
	    (BN_mod(rsa->dmp1, rsa->d, aux, ctx) == 0))
		fatal("rsa_generate_additional_parameters: BN_sub/mod failed");

	BN_clear_free(aux);
	BN_CTX_free(ctx);
}