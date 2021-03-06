
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

#include "hx_locl.h"

/**
 * @page page_peer Hx509 crypto selecting functions
 *
 * Peer info structures are used togeter with hx509_crypto_select() to
 * select the best avaible crypto algorithm to use.
 *
 * See the library functions here: @ref hx509_peer
 */

/**
 * Allocate a new peer info structure an init it to default values.
 *
 * @param context A hx509 context.
 * @param peer return an allocated peer, free with hx509_peer_info_free().
 *
 * @return An hx509 error code, see hx509_get_error_string().
 *
 * @ingroup hx509_peer
 */

int
hx509_peer_info_alloc(hx509_context context, hx509_peer_info *peer)
{
    *peer = calloc(1, sizeof(**peer));
    if (*peer == NULL) {
	hx509_set_error_string(context, 0, ENOMEM, "out of memory");
	return ENOMEM;
    }
    return 0;
}


static void
free_cms_alg(hx509_peer_info peer)
{
    if (peer->val) {
	size_t i;
	for (i = 0; i < peer->len; i++)
	    free_AlgorithmIdentifier(&peer->val[i]);
	free(peer->val);
	peer->val = NULL;
	peer->len = 0;
    }
}

/**
 * Free a peer info structure.
 *
 * @param peer peer info to be freed.
 *
 * @ingroup hx509_peer
 */

void
hx509_peer_info_free(hx509_peer_info peer)
{
    if (peer == NULL)
	return;
    if (peer->cert)
	hx509_cert_free(peer->cert);
    free_cms_alg(peer);
    memset(peer, 0, sizeof(*peer));
    free(peer);
}

/**
 * Set the certificate that remote peer is using.
 *
 * @param peer peer info to update
 * @param cert cerificate of the remote peer.
 *
 * @return An hx509 error code, see hx509_get_error_string().
 *
 * @ingroup hx509_peer
 */

int
hx509_peer_info_set_cert(hx509_peer_info peer,
			 hx509_cert cert)
{
    if (peer->cert)
	hx509_cert_free(peer->cert);
    peer->cert = hx509_cert_ref(cert);
    return 0;
}

/**
 * Add an additional algorithm that the peer supports.
 *
 * @param context A hx509 context.
 * @param peer the peer to set the new algorithms for
 * @param val an AlgorithmsIdentier to add
 *
 * @return An hx509 error code, see hx509_get_error_string().
 *
 * @ingroup hx509_peer
 */

int
hx509_peer_info_add_cms_alg(hx509_context context,
			    hx509_peer_info peer,
			    const AlgorithmIdentifier *val)
{
    void *ptr;
    int ret;

    ptr = realloc(peer->val, sizeof(peer->val[0]) * (peer->len + 1));
    if (ptr == NULL) {
	hx509_set_error_string(context, 0, ENOMEM, "out of memory");
	return ENOMEM;
    }
    peer->val = ptr;
    ret = copy_AlgorithmIdentifier(val, &peer->val[peer->len]);
    if (ret == 0)
	peer->len += 1;
    else
	hx509_set_error_string(context, 0, ret, "out of memory");
    return ret;
}

/**
 * Set the algorithms that the peer supports.
 *
 * @param context A hx509 context.
 * @param peer the peer to set the new algorithms for
 * @param val array of supported AlgorithmsIdentiers
 * @param len length of array val.
 *
 * @return An hx509 error code, see hx509_get_error_string().
 *
 * @ingroup hx509_peer
 */

int
hx509_peer_info_set_cms_algs(hx509_context context,
			     hx509_peer_info peer,
			     const AlgorithmIdentifier *val,
			     size_t len)
{
    size_t i;

    free_cms_alg(peer);

    peer->val = calloc(len, sizeof(*peer->val));
    if (peer->val == NULL) {
	peer->len = 0;
	hx509_set_error_string(context, 0, ENOMEM, "out of memory");
	return ENOMEM;
    }
    peer->len = len;
    for (i = 0; i < len; i++) {
	int ret;
	ret = copy_AlgorithmIdentifier(&val[i], &peer->val[i]);
	if (ret) {
	    hx509_clear_error_string(context);
	    free_cms_alg(peer);
	    return ret;
	}
    }
    return 0;
}

#if 0

/*
 * S/MIME
 */

int
hx509_peer_info_parse_smime(hx509_peer_info peer,
			    const heim_octet_string *data)
{
    return 0;
}

int
hx509_peer_info_unparse_smime(hx509_peer_info peer,
			      heim_octet_string *data)
{
    return 0;
}

/*
 * For storing hx509_peer_info to be able to cache them.
 */

int
hx509_peer_info_parse(hx509_peer_info peer,
		      const heim_octet_string *data)
{
    return 0;
}

int
hx509_peer_info_unparse(hx509_peer_info peer,
			heim_octet_string *data)
{
    return 0;
}
#endif