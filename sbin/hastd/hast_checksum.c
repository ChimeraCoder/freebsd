
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

#include <errno.h>
#include <string.h>
#include <strings.h>

#ifdef HAVE_CRYPTO
#include <openssl/sha.h>
#endif

#include <crc32.h>
#include <hast.h>
#include <nv.h>
#include <pjdlog.h>

#include "hast_checksum.h"

#ifdef HAVE_CRYPTO
#define	MAX_HASH_SIZE	SHA256_DIGEST_LENGTH
#else
#define	MAX_HASH_SIZE	4
#endif

static void
hast_crc32_checksum(const unsigned char *data, size_t size,
    unsigned char *hash, size_t *hsizep)
{
	uint32_t crc;

	crc = crc32(data, size);
	/* XXXPJD: Do we have to use htole32() on crc first? */
	bcopy(&crc, hash, sizeof(crc));
	*hsizep = sizeof(crc);
}

#ifdef HAVE_CRYPTO
static void
hast_sha256_checksum(const unsigned char *data, size_t size,
    unsigned char *hash, size_t *hsizep)
{
	SHA256_CTX ctx;

	SHA256_Init(&ctx);
	SHA256_Update(&ctx, data, size);
	SHA256_Final(hash, &ctx);
	*hsizep = SHA256_DIGEST_LENGTH;
}
#endif	/* HAVE_CRYPTO */

const char *
checksum_name(int num)
{

	switch (num) {
	case HAST_CHECKSUM_NONE:
		return ("none");
	case HAST_CHECKSUM_CRC32:
		return ("crc32");
	case HAST_CHECKSUM_SHA256:
		return ("sha256");
	}
	return ("unknown");
}

int
checksum_send(const struct hast_resource *res, struct nv *nv, void **datap,
    size_t *sizep, bool *freedatap __unused)
{
	unsigned char hash[MAX_HASH_SIZE];
	size_t hsize;

	switch (res->hr_checksum) {
	case HAST_CHECKSUM_NONE:
		return (0);
	case HAST_CHECKSUM_CRC32:
		hast_crc32_checksum(*datap, *sizep, hash, &hsize);
		break;
#ifdef HAVE_CRYPTO
	case HAST_CHECKSUM_SHA256:
		hast_sha256_checksum(*datap, *sizep, hash, &hsize);
		break;
#endif
	default:
		PJDLOG_ABORT("Invalid checksum: %d.", res->hr_checksum);
	}
	nv_add_string(nv, checksum_name(res->hr_checksum), "checksum");
	nv_add_uint8_array(nv, hash, hsize, "hash");
	if (nv_error(nv) != 0) {
		errno = nv_error(nv);
		return (-1);
	}
	return (0);
}

int
checksum_recv(const struct hast_resource *res __unused, struct nv *nv,
    void **datap, size_t *sizep, bool *freedatap __unused)
{
	unsigned char chash[MAX_HASH_SIZE];
	const unsigned char *rhash;
	size_t chsize, rhsize;
	const char *algo;

	algo = nv_get_string(nv, "checksum");
	if (algo == NULL)
		return (0);	/* No checksum. */
	rhash = nv_get_uint8_array(nv, &rhsize, "hash");
	if (rhash == NULL) {
		pjdlog_error("Hash is missing.");
		return (-1);	/* Hash not found. */
	}
	if (strcmp(algo, "crc32") == 0)
		hast_crc32_checksum(*datap, *sizep, chash, &chsize);
#ifdef HAVE_CRYPTO
	else if (strcmp(algo, "sha256") == 0)
		hast_sha256_checksum(*datap, *sizep, chash, &chsize);
#endif
	else {
		pjdlog_error("Unknown checksum algorithm '%s'.", algo);
		return (-1);	/* Unknown checksum algorithm. */
	}
	if (rhsize != chsize) {
		pjdlog_error("Invalid hash size (%zu) for %s, should be %zu.",
		    rhsize, algo, chsize);
		return (-1);	/* Different hash size. */
	}
	if (bcmp(rhash, chash, chsize) != 0) {
		pjdlog_error("Hash mismatch.");
		return (-1);	/* Hash mismatch. */
	}

	return (0);
}