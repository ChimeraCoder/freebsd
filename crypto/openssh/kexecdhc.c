
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

#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "xmalloc.h"
#include "buffer.h"
#include "key.h"
#include "cipher.h"
#include "kex.h"
#include "log.h"
#include "packet.h"
#include "dh.h"
#include "ssh2.h"

#ifdef OPENSSL_HAS_ECC

#include <openssl/ecdh.h>

void
kexecdh_client(Kex *kex)
{
	EC_KEY *client_key;
	EC_POINT *server_public;
	const EC_GROUP *group;
	BIGNUM *shared_secret;
	Key *server_host_key;
	u_char *server_host_key_blob = NULL, *signature = NULL;
	u_char *kbuf, *hash;
	u_int klen, slen, sbloblen, hashlen;
	int curve_nid;

	if ((curve_nid = kex_ecdh_name_to_nid(kex->name)) == -1)
		fatal("%s: unsupported ECDH curve \"%s\"", __func__, kex->name);
	if ((client_key = EC_KEY_new_by_curve_name(curve_nid)) == NULL)
		fatal("%s: EC_KEY_new_by_curve_name failed", __func__);
	if (EC_KEY_generate_key(client_key) != 1)
		fatal("%s: EC_KEY_generate_key failed", __func__);
	group = EC_KEY_get0_group(client_key);

	packet_start(SSH2_MSG_KEX_ECDH_INIT);
	packet_put_ecpoint(group, EC_KEY_get0_public_key(client_key));
	packet_send();
	debug("sending SSH2_MSG_KEX_ECDH_INIT");

#ifdef DEBUG_KEXECDH
	fputs("client private key:\n", stderr);
	key_dump_ec_key(client_key);
#endif

	debug("expecting SSH2_MSG_KEX_ECDH_REPLY");
	packet_read_expect(SSH2_MSG_KEX_ECDH_REPLY);

	/* hostkey */
	server_host_key_blob = packet_get_string(&sbloblen);
	server_host_key = key_from_blob(server_host_key_blob, sbloblen);
	if (server_host_key == NULL)
		fatal("cannot decode server_host_key_blob");
	if (server_host_key->type != kex->hostkey_type)
		fatal("type mismatch for decoded server_host_key_blob");
	if (kex->verify_host_key == NULL)
		fatal("cannot verify server_host_key");
	if (kex->verify_host_key(server_host_key) == -1)
		fatal("server_host_key verification failed");

	/* Q_S, server public key */
	if ((server_public = EC_POINT_new(group)) == NULL)
		fatal("%s: EC_POINT_new failed", __func__);
	packet_get_ecpoint(group, server_public);

	if (key_ec_validate_public(group, server_public) != 0)
		fatal("%s: invalid server public key", __func__);

#ifdef DEBUG_KEXECDH
	fputs("server public key:\n", stderr);
	key_dump_ec_point(group, server_public);
#endif

	/* signed H */
	signature = packet_get_string(&slen);
	packet_check_eom();

	klen = (EC_GROUP_get_degree(group) + 7) / 8;
	kbuf = xmalloc(klen);
	if (ECDH_compute_key(kbuf, klen, server_public,
	    client_key, NULL) != (int)klen)
		fatal("%s: ECDH_compute_key failed", __func__);

#ifdef DEBUG_KEXECDH
	dump_digest("shared secret", kbuf, klen);
#endif
	if ((shared_secret = BN_new()) == NULL)
		fatal("%s: BN_new failed", __func__);
	if (BN_bin2bn(kbuf, klen, shared_secret) == NULL)
		fatal("%s: BN_bin2bn failed", __func__);
	memset(kbuf, 0, klen);
	xfree(kbuf);

	/* calc and verify H */
	kex_ecdh_hash(
	    kex->evp_md,
	    group,
	    kex->client_version_string,
	    kex->server_version_string,
	    buffer_ptr(&kex->my), buffer_len(&kex->my),
	    buffer_ptr(&kex->peer), buffer_len(&kex->peer),
	    server_host_key_blob, sbloblen,
	    EC_KEY_get0_public_key(client_key),
	    server_public,
	    shared_secret,
	    &hash, &hashlen
	);
	xfree(server_host_key_blob);
	EC_POINT_clear_free(server_public);
	EC_KEY_free(client_key);

	if (key_verify(server_host_key, signature, slen, hash, hashlen) != 1)
		fatal("key_verify failed for server_host_key");
	key_free(server_host_key);
	xfree(signature);

	/* save session id */
	if (kex->session_id == NULL) {
		kex->session_id_len = hashlen;
		kex->session_id = xmalloc(kex->session_id_len);
		memcpy(kex->session_id, hash, kex->session_id_len);
	}

	kex_derive_keys(kex, hash, hashlen, shared_secret);
	BN_clear_free(shared_secret);
	kex_finish(kex);
}
#else /* OPENSSL_HAS_ECC */
void
kexecdh_client(Kex *kex)
{
	fatal("ECC support is not enabled");
}
#endif /* OPENSSL_HAS_ECC */