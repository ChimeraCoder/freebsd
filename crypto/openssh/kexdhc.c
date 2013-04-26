
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

#include <openssl/dh.h>

#include <stdarg.h>
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

void
kexdh_client(Kex *kex)
{
	BIGNUM *dh_server_pub = NULL, *shared_secret = NULL;
	DH *dh;
	Key *server_host_key;
	u_char *server_host_key_blob = NULL, *signature = NULL;
	u_char *kbuf, *hash;
	u_int klen, slen, sbloblen, hashlen;
	int kout;

	/* generate and send 'e', client DH public key */
	switch (kex->kex_type) {
	case KEX_DH_GRP1_SHA1:
		dh = dh_new_group1();
		break;
	case KEX_DH_GRP14_SHA1:
		dh = dh_new_group14();
		break;
	default:
		fatal("%s: Unexpected KEX type %d", __func__, kex->kex_type);
	}
	dh_gen_key(dh, kex->we_need * 8);
	packet_start(SSH2_MSG_KEXDH_INIT);
	packet_put_bignum2(dh->pub_key);
	packet_send();

	debug("sending SSH2_MSG_KEXDH_INIT");
#ifdef DEBUG_KEXDH
	DHparams_print_fp(stderr, dh);
	fprintf(stderr, "pub= ");
	BN_print_fp(stderr, dh->pub_key);
	fprintf(stderr, "\n");
#endif

	debug("expecting SSH2_MSG_KEXDH_REPLY");
	packet_read_expect(SSH2_MSG_KEXDH_REPLY);

	/* key, cert */
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

	/* DH parameter f, server public DH key */
	if ((dh_server_pub = BN_new()) == NULL)
		fatal("dh_server_pub == NULL");
	packet_get_bignum2(dh_server_pub);

#ifdef DEBUG_KEXDH
	fprintf(stderr, "dh_server_pub= ");
	BN_print_fp(stderr, dh_server_pub);
	fprintf(stderr, "\n");
	debug("bits %d", BN_num_bits(dh_server_pub));
#endif

	/* signed H */
	signature = packet_get_string(&slen);
	packet_check_eom();

	if (!dh_pub_is_valid(dh, dh_server_pub))
		packet_disconnect("bad server public DH value");

	klen = DH_size(dh);
	kbuf = xmalloc(klen);
	if ((kout = DH_compute_key(kbuf, dh_server_pub, dh)) < 0)
		fatal("DH_compute_key: failed");
#ifdef DEBUG_KEXDH
	dump_digest("shared secret", kbuf, kout);
#endif
	if ((shared_secret = BN_new()) == NULL)
		fatal("kexdh_client: BN_new failed");
	if (BN_bin2bn(kbuf, kout, shared_secret) == NULL)
		fatal("kexdh_client: BN_bin2bn failed");
	memset(kbuf, 0, klen);
	xfree(kbuf);

	/* calc and verify H */
	kex_dh_hash(
	    kex->client_version_string,
	    kex->server_version_string,
	    buffer_ptr(&kex->my), buffer_len(&kex->my),
	    buffer_ptr(&kex->peer), buffer_len(&kex->peer),
	    server_host_key_blob, sbloblen,
	    dh->pub_key,
	    dh_server_pub,
	    shared_secret,
	    &hash, &hashlen
	);
	xfree(server_host_key_blob);
	BN_clear_free(dh_server_pub);
	DH_free(dh);

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