
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

#include <sys/cdefs.h>

#include <sys/types.h>
#ifdef _KERNEL
#include <sys/systm.h>
#endif
#include <crypto/camellia/camellia.h>

void
camellia_set_key(camellia_ctx *ctx, const u_char *key, int bits)
{

    Camellia_Ekeygen(bits, key, ctx->subkey);
    ctx->bits = bits;
}

void
camellia_decrypt(const camellia_ctx *ctx, const u_char *src, u_char *dst)
{

    Camellia_DecryptBlock(ctx->bits, src, ctx->subkey, dst);
}

void
camellia_encrypt(const camellia_ctx *ctx, const u_char *src, u_char *dst)
{

    Camellia_EncryptBlock(ctx->bits, src, ctx->subkey, dst);
}