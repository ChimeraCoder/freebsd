
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
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/md5.h>
#include <sys/kobj.h>
#include <sys/mbuf.h>
#include <crypto/rc4/rc4.h>

#include <kgssapi/gssapi.h>
#include <kgssapi/gssapi_impl.h>

#include "kcrypto.h"

static void
arcfour_init(struct krb5_key_state *ks)
{

	ks->ks_priv = NULL;
}

static void
arcfour_destroy(struct krb5_key_state *ks)
{

}

static void
arcfour_set_key(struct krb5_key_state *ks, const void *in)
{
	void *kp = ks->ks_key;

	if (kp != in)
		bcopy(in, kp, 16);
}

static void
arcfour_random_to_key(struct krb5_key_state *ks, const void *in)
{

	arcfour_set_key(ks, in);
}

static void
arcfour_hmac(uint8_t *key, uint8_t *data, size_t datalen,
	uint8_t *result)
{
	uint8_t buf[64];
	MD5_CTX md5;
	int i;

	for (i = 0; i < 16; i++)
		buf[i] = key[i] ^ 0x36;
	for (; i < 64; i++)
		buf[i] = 0x36;

	MD5Init(&md5);
	MD5Update(&md5, buf, 64);
	MD5Update(&md5, data, datalen);
	MD5Final(result, &md5);

	for (i = 0; i < 16; i++)
		buf[i] = key[i] ^ 0x5c;
	for (; i < 64; i++)
		buf[i] = 0x5c;
	
	MD5Init(&md5);
	MD5Update(&md5, buf, 64);
	MD5Update(&md5, result, 16);
	MD5Final(result, &md5);
}

static void
arcfour_derive_key(const struct krb5_key_state *ks, uint32_t usage,
    uint8_t *newkey)
{
	uint8_t t[4];

	t[0] = (usage >> 24);
	t[1] = (usage >> 16);
	t[2] = (usage >> 8);
	t[3] = (usage >> 0);
	if (ks->ks_class->ec_type == ETYPE_ARCFOUR_HMAC_MD5_56) {
		uint8_t L40[14] = "fortybits";
		bcopy(t, L40 + 10, 4);
		arcfour_hmac(ks->ks_key, L40, 14, newkey);
		memset(newkey + 7, 0xab, 9);
	} else {
		arcfour_hmac(ks->ks_key, t, 4, newkey);
	}
}

static int
rc4_crypt_int(void *rs, void *buf, u_int len)
{

	rc4_crypt(rs, buf, buf, len);
	return (0);
}

static void
arcfour_encrypt(const struct krb5_key_state *ks, struct mbuf *inout,
    size_t skip, size_t len, void *ivec, size_t ivlen)
{
	struct rc4_state rs;
	uint8_t newkey[16];

	arcfour_derive_key(ks, 0, newkey);

	/*
	 * If we have an IV, then generate a new key from it using HMAC.
	 */
	if (ivec) {
		uint8_t kk[16];
		arcfour_hmac(newkey, ivec, ivlen, kk);
		rc4_init(&rs, kk, 16);
	} else {
		rc4_init(&rs, newkey, 16);
	}

	m_apply(inout, skip, len, rc4_crypt_int, &rs);
}

static int
MD5Update_int(void *ctx, void *buf, u_int len)
{

	MD5Update(ctx, buf, len);
	return (0);
}

static void
arcfour_checksum(const struct krb5_key_state *ks, int usage,
    struct mbuf *inout, size_t skip, size_t inlen, size_t outlen)
{
	MD5_CTX md5;
	uint8_t Ksign[16];
	uint8_t t[4];
	uint8_t sgn_cksum[16];

	arcfour_hmac(ks->ks_key, "signaturekey", 13, Ksign);

	t[0] = usage >> 0;
	t[1] = usage >> 8;
	t[2] = usage >> 16;
	t[3] = usage >> 24;

	MD5Init(&md5);
	MD5Update(&md5, t, 4);
	m_apply(inout, skip, inlen, MD5Update_int, &md5);
	MD5Final(sgn_cksum, &md5);
	
	arcfour_hmac(Ksign, sgn_cksum, 16, sgn_cksum);
	m_copyback(inout, skip + inlen, outlen, sgn_cksum);
}

struct krb5_encryption_class krb5_arcfour_encryption_class = {
	"arcfour-hmac-md5",	/* name */
	ETYPE_ARCFOUR_HMAC_MD5, /* etype */
	0,			/* flags */
	1,			/* blocklen */
	1,			/* msgblocklen */
	8,			/* checksumlen */
	128,			/* keybits */
	16,			/* keylen */
	arcfour_init,
	arcfour_destroy,
	arcfour_set_key,
	arcfour_random_to_key,
	arcfour_encrypt,
	arcfour_encrypt,
	arcfour_checksum
};

struct krb5_encryption_class krb5_arcfour_56_encryption_class = {
	"arcfour-hmac-md5-56",	   /* name */
	ETYPE_ARCFOUR_HMAC_MD5_56, /* etype */
	0,			   /* flags */
	1,			/* blocklen */
	1,			/* msgblocklen */
	8,			/* checksumlen */
	128,			/* keybits */
	16,			/* keylen */
	arcfour_init,
	arcfour_destroy,
	arcfour_set_key,
	arcfour_random_to_key,
	arcfour_encrypt,
	arcfour_encrypt,
	arcfour_checksum
};