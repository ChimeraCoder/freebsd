
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
#include "compat.h"

void
kexgex_client(Kex *kex)
{
	BIGNUM *dh_server_pub = NULL, *shared_secret = NULL;
	BIGNUM *p = NULL, *g = NULL;
	Key *server_host_key;
	u_char *kbuf, *hash, *signature = NULL, *server_host_key_blob = NULL;
	u_int klen, slen, sbloblen, hashlen;
	int kout;
	int min, max, nbits;
	DH *dh;

	nbits = dh_estimate(kex->we_need * 8);

	if (datafellows & SSH_OLD_DHGEX) {
		/* Old GEX request */
		packet_start(SSH2_MSG_KEX_DH_GEX_REQUEST_OLD);
		packet_put_int(nbits);
		min = DH_GRP_MIN;
		max = DH_GRP_MAX;

		debug("SSH2_MSG_KEX_DH_GEX_REQUEST_OLD(%u) sent", nbits);
	} else {
		/* New GEX request */
		min = DH_GRP_MIN;
		max = DH_GRP_MAX;
		packet_start(SSH2_MSG_KEX_DH_GEX_REQUEST);
		packet_put_int(min);
		packet_put_int(nbits);
		packet_put_int(max);

		debug("SSH2_MSG_KEX_DH_GEX_REQUEST(%u<%u<%u) sent",
		    min, nbits, max);
	}
#ifdef DEBUG_KEXDH
	fprintf(stderr, "\nmin = %d, nbits = %d, max = %d\n",
	    min, nbits, max);
#endif
	packet_send();

	debug("expecting SSH2_MSG_KEX_DH_GEX_GROUP");
	packet_read_expect(SSH2_MSG_KEX_DH_GEX_GROUP);

	if ((p = BN_new()) == NULL)
		fatal("BN_new");
	packet_get_bignum2(p);
	if ((g = BN_new()) == NULL)
		fatal("BN_new");
	packet_get_bignum2(g);
	packet_check_eom();

	if (BN_num_bits(p) < min || BN_num_bits(p) > max)
		fatal("DH_GEX group out of range: %d !< %d !< %d",
		    min, BN_num_bits(p), max);

	dh = dh_new_group(g, p);
	dh_gen_key(dh, kex->we_need * 8);

#ifdef DEBUG_KEXDH
	DHparams_print_fp(stderr, dh);
	fprintf(stderr, "pub= ");
	BN_print_fp(stderr, dh->pub_key);
	fprintf(stderr, "\n");
#endif

	debug("SSH2_MSG_KEX_DH_GEX_INIT sent");
	/* generate and send 'e', client DH public key */
	packet_start(SSH2_MSG_KEX_DH_GEX_INIT);
	packet_put_bignum2(dh->pub_key);
	packet_send();

	debug("expecting SSH2_MSG_KEX_DH_GEX_REPLY");
	packet_read_expect(SSH2_MSG_KEX_DH_GEX_REPLY);

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
		fatal("kexgex_client: BN_new failed");
	if (BN_bin2bn(kbuf, kout, shared_secret) == NULL)
		fatal("kexgex_client: BN_bin2bn failed");
	memset(kbuf, 0, klen);
	xfree(kbuf);

	if (datafellows & SSH_OLD_DHGEX)
		min = max = -1;

	/* calc and verify H */
	kexgex_hash(
	    kex->evp_md,
	    kex->client_version_string,
	    kex->server_version_string,
	    buffer_ptr(&kex->my), buffer_len(&kex->my),
	    buffer_ptr(&kex->peer), buffer_len(&kex->peer),
	    server_host_key_blob, sbloblen,
	    min, nbits, max,
	    dh->p, dh->g,
	    dh->pub_key,
	    dh_server_pub,
	    shared_secret,
	    &hash, &hashlen
	);

	/* have keys, free DH */
	DH_free(dh);
	xfree(server_host_key_blob);
	BN_clear_free(dh_server_pub);

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