
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
#include <sys/zfs_context.h>
#include <sys/zio.h>
#ifdef _KERNEL
#include <crypto/sha2/sha2.h>
#else
#include <sha256.h>
#endif

void
zio_checksum_SHA256(const void *buf, uint64_t size, zio_cksum_t *zcp)
{
	SHA256_CTX ctx;
	zio_cksum_t tmp;

	SHA256_Init(&ctx);
	SHA256_Update(&ctx, buf, size);
	SHA256_Final((unsigned char *)&tmp, &ctx);

	/*
	 * A prior implementation of this function had a
	 * private SHA256 implementation always wrote things out in
	 * Big Endian and there wasn't a byteswap variant of it.
	 * To preseve on disk compatibility we need to force that
	 * behaviour.
	 */
	zcp->zc_word[0] = BE_64(tmp.zc_word[0]);
	zcp->zc_word[1] = BE_64(tmp.zc_word[1]);
	zcp->zc_word[2] = BE_64(tmp.zc_word[2]);
	zcp->zc_word[3] = BE_64(tmp.zc_word[3]);
}