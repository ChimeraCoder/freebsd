
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
#ifdef GSSAPI
#include "ssh-gss.h"
#endif
#include "monitor_wrap.h"

#ifdef OPENSSL_HAS_ECC

#include <openssl/ecdh.h>

void
kexecdh_server(Kex *kex)
{
	EC_POINT *client_public;
	EC_KEY *server_key;
	const EC_GROUP *group;
	BIGNUM *shared_secret;
	Key *server_host_private, *server_host_public;
	u_char *server_host_key_blob = NULL, *signature = NULL;
	u_char *kbuf, *hash;
	u_int klen, slen, sbloblen, hashlen;
	int curve_nid;

	if ((curve_nid = kex_ecdh_name_to_nid(kex->name)) == -1)
		fatal("%s: unsupported ECDH curve \"%s\"", __func__, kex->name);
	if ((server_key = EC_KEY_new_by_curve_name(curve_nid)) == NULL)
		fatal("%s: EC_KEY_new_by_curve_name failed", __func__);
	if (EC_KEY_generate_key(server_key) != 1)
		fatal("%s: EC_KEY_generate_key failed", __func__);
	group = EC_KEY_get0_group(server_key);

#ifdef DEBUG_KEXECDH
	fputs("server private key:\n", stderr);
	key_dump_ec_key(server_key);
#endif

	if (kex->load_host_public_key == NULL ||
	    kex->load_host_private_key == NULL)
		fatal("Cannot load hostkey");
	server_host_public = kex->load_host_public_key(kex->hostkey_type);
	if (server_host_public == NULL)
		fatal("Unsupported hostkey type %d", kex->hostkey_type);
	server_host_private = kex->load_host_private_key(kex->hostkey_type);
	if (server_host_private == NULL)
		fatal("Missing private key for hostkey type %d",
		    kex->hostkey_type);

	debug("expecting SSH2_MSG_KEX_ECDH_INIT");
	packet_read_expect(SSH2_MSG_KEX_ECDH_INIT);
	if ((client_public = EC_POINT_new(group)) == NULL)
		fatal("%s: EC_POINT_new failed", __func__);
	packet_get_ecpoint(group, client_public);
	packet_check_eom();

	if (key_ec_validate_public(group, client_public) != 0)
		fatal("%s: invalid client public key", __func__);

#ifdef DEBUG_KEXECDH
	fputs("client public key:\n", stderr);
	key_dump_ec_point(group, client_public);
#endif

	/* Calculate shared_secret */
	klen = (EC_GROUP_get_degree(group) + 7) / 8;
	kbuf = xmalloc(klen);
	if (ECDH_compute_key(kbuf, klen, client_public,
	    server_key, NULL) != (int)klen)
		fatal("%s: ECDH_compute_key failed", __func__);

#ifdef DEBUG_KEXDH
	dump_digest("shared secret", kbuf, klen);
#endif
	if ((shared_secret = BN_new()) == NULL)
		fatal("%s: BN_new failed", __func__);
	if (BN_bin2bn(kbuf, klen, shared_secret) == NULL)
		fatal("%s: BN_bin2bn failed", __func__);
	memset(kbuf, 0, klen);
	xfree(kbuf);

	/* calc H */
	key_to_blob(server_host_public, &server_host_key_blob, &sbloblen);
	kex_ecdh_hash(
	    kex->evp_md,
	    group,
	    kex->client_version_string,
	    kex->server_version_string,
	    buffer_ptr(&kex->peer), buffer_len(&kex->peer),
	    buffer_ptr(&kex->my), buffer_len(&kex->my),
	    server_host_key_blob, sbloblen,
	    client_public,
	    EC_KEY_get0_public_key(server_key),
	    shared_secret,
	    &hash, &hashlen
	);
	EC_POINT_clear_free(client_public);

	/* save session id := H */
	if (kex->session_id == NULL) {
		kex->session_id_len = hashlen;
		kex->session_id = xmalloc(kex->session_id_len);
		memcpy(kex->session_id, hash, kex->session_id_len);
	}

	/* sign H */
	if (PRIVSEP(key_sign(server_host_private, &signature, &slen,
	    hash, hashlen)) < 0)
		fatal("kexdh_server: key_sign failed");

	/* destroy_sensitive_data(); */

	/* send server hostkey, ECDH pubkey 'Q_S' and signed H */
	packet_start(SSH2_MSG_KEX_ECDH_REPLY);
	packet_put_string(server_host_key_blob, sbloblen);
	packet_put_ecpoint(group, EC_KEY_get0_public_key(server_key));
	packet_put_string(signature, slen);
	packet_send();

	xfree(signature);
	xfree(server_host_key_blob);
	/* have keys, free server key */
	EC_KEY_free(server_key);

	kex_derive_keys(kex, hash, hashlen, shared_secret);
	BN_clear_free(shared_secret);
	kex_finish(kex);
}
#else /* OPENSSL_HAS_ECC */
void
kexecdh_server(Kex *kex)
{
	fatal("ECC support is not enabled");
}
#endif /* OPENSSL_HAS_ECC */