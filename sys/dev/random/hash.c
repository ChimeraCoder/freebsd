
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
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>

#include <crypto/rijndael/rijndael-api-fst.h>
#include <crypto/sha2/sha2.h>

#include <dev/random/hash.h>

/* initialise the hash */
void
yarrow_hash_init(struct yarrowhash *context)
{
	SHA256_Init(&context->sha);
}

/* iterate the hash */
void
yarrow_hash_iterate(struct yarrowhash *context, void *data, size_t size)
{
	SHA256_Update(&context->sha, data, size);
}

/* Conclude by returning the hash in the supplied /buf/ which must be
 * KEYSIZE bytes long.
 */
void
yarrow_hash_finish(struct yarrowhash *context, void *buf)
{
	SHA256_Final(buf, &context->sha);
}

/* Initialise the encryption routine by setting up the key schedule
 * from the supplied /data/ which must be KEYSIZE bytes of binary
 * data.
 */
void
yarrow_encrypt_init(struct yarrowkey *context, void *data)
{
	rijndael_cipherInit(&context->cipher, MODE_CBC, NULL);
	rijndael_makeKey(&context->key, DIR_ENCRYPT, KEYSIZE*8, data);
}

/* Encrypt the supplied data using the key schedule preset in the context.
 * KEYSIZE bytes are encrypted from /d_in/ to /d_out/.
 */
void
yarrow_encrypt(struct yarrowkey *context, void *d_in, void *d_out)
{
	rijndael_blockEncrypt(&context->cipher, &context->key, d_in,
	    KEYSIZE*8, d_out);
}