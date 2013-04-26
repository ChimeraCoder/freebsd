
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

#include <openssl/asn1.h>
#include <openssl/asn1t.h>
#include <openssl/bio.h>
#include <openssl/err.h>

#include <stdio.h>

/* Experimental NDEF ASN1 BIO support routines */

/* The usage is quite simple, initialize an ASN1 structure,
 * get a BIO from it then any data written through the BIO
 * will end up translated to approptiate format on the fly.
 * The data is streamed out and does *not* need to be
 * all held in memory at once.
 *
 * When the BIO is flushed the output is finalized and any
 * signatures etc written out.
 *
 * The BIO is a 'proper' BIO and can handle non blocking I/O
 * correctly.
 *
 * The usage is simple. The implementation is *not*...
 */

/* BIO support data stored in the ASN1 BIO ex_arg */

typedef struct ndef_aux_st
	{
	/* ASN1 structure this BIO refers to */
	ASN1_VALUE *val;
	const ASN1_ITEM *it;
	/* Top of the BIO chain */
	BIO *ndef_bio;
	/* Output BIO */
	BIO *out;
	/* Boundary where content is inserted */
	unsigned char **boundary;
	/* DER buffer start */
	unsigned char *derbuf;
	} NDEF_SUPPORT;

static int ndef_prefix(BIO *b, unsigned char **pbuf, int *plen, void *parg);
static int ndef_prefix_free(BIO *b, unsigned char **pbuf, int *plen, void *parg);
static int ndef_suffix(BIO *b, unsigned char **pbuf, int *plen, void *parg);
static int ndef_suffix_free(BIO *b, unsigned char **pbuf, int *plen, void *parg);

BIO *BIO_new_NDEF(BIO *out, ASN1_VALUE *val, const ASN1_ITEM *it)
	{
	NDEF_SUPPORT *ndef_aux = NULL;
	BIO *asn_bio = NULL;
	const ASN1_AUX *aux = it->funcs;
	ASN1_STREAM_ARG sarg;

	if (!aux || !aux->asn1_cb)
		{
		ASN1err(ASN1_F_BIO_NEW_NDEF, ASN1_R_STREAMING_NOT_SUPPORTED);
		return NULL;
		}
	ndef_aux = OPENSSL_malloc(sizeof(NDEF_SUPPORT));
	asn_bio = BIO_new(BIO_f_asn1());

	/* ASN1 bio needs to be next to output BIO */

	out = BIO_push(asn_bio, out);

	if (!ndef_aux || !asn_bio || !out)
		goto err;

	BIO_asn1_set_prefix(asn_bio, ndef_prefix, ndef_prefix_free);
	BIO_asn1_set_suffix(asn_bio, ndef_suffix, ndef_suffix_free);

	/* Now let callback prepend any digest, cipher etc BIOs
	 * ASN1 structure needs.
	 */

	sarg.out = out;
	sarg.ndef_bio = NULL;
	sarg.boundary = NULL;

	if (aux->asn1_cb(ASN1_OP_STREAM_PRE, &val, it, &sarg) <= 0)
		goto err;

	ndef_aux->val = val;
	ndef_aux->it = it;
	ndef_aux->ndef_bio = sarg.ndef_bio;
	ndef_aux->boundary = sarg.boundary;
	ndef_aux->out = out;

	BIO_ctrl(asn_bio, BIO_C_SET_EX_ARG, 0, ndef_aux);

	return sarg.ndef_bio;

	err:
	if (asn_bio)
		BIO_free(asn_bio);
	if (ndef_aux)
		OPENSSL_free(ndef_aux);
	return NULL;
	}

static int ndef_prefix(BIO *b, unsigned char **pbuf, int *plen, void *parg)
	{
	NDEF_SUPPORT *ndef_aux;
	unsigned char *p;
	int derlen;

	if (!parg)
		return 0;

	ndef_aux = *(NDEF_SUPPORT **)parg;

	derlen = ASN1_item_ndef_i2d(ndef_aux->val, NULL, ndef_aux->it);
	p = OPENSSL_malloc(derlen);
	ndef_aux->derbuf = p;
	*pbuf = p;
	derlen = ASN1_item_ndef_i2d(ndef_aux->val, &p, ndef_aux->it);

	if (!*ndef_aux->boundary)
		return 0;

	*plen = *ndef_aux->boundary - *pbuf;

	return 1;
	}

static int ndef_prefix_free(BIO *b, unsigned char **pbuf, int *plen, void *parg)
	{
	NDEF_SUPPORT *ndef_aux;

	if (!parg)
		return 0;

	ndef_aux = *(NDEF_SUPPORT **)parg;

	if (ndef_aux->derbuf)
		OPENSSL_free(ndef_aux->derbuf);

	ndef_aux->derbuf = NULL;
	*pbuf = NULL;
	*plen = 0;
	return 1;
	}

static int ndef_suffix_free(BIO *b, unsigned char **pbuf, int *plen, void *parg)
	{
	NDEF_SUPPORT **pndef_aux = (NDEF_SUPPORT **)parg;
	if (!ndef_prefix_free(b, pbuf, plen, parg))
		return 0;
	OPENSSL_free(*pndef_aux);
	*pndef_aux = NULL;
	return 1;
	}

static int ndef_suffix(BIO *b, unsigned char **pbuf, int *plen, void *parg)
	{
	NDEF_SUPPORT *ndef_aux;
	unsigned char *p;
	int derlen;
	const ASN1_AUX *aux;
	ASN1_STREAM_ARG sarg;

	if (!parg)
		return 0;

	ndef_aux = *(NDEF_SUPPORT **)parg;

	aux = ndef_aux->it->funcs;

	/* Finalize structures */
	sarg.ndef_bio = ndef_aux->ndef_bio;
	sarg.out = ndef_aux->out;
	sarg.boundary = ndef_aux->boundary;
	if (aux->asn1_cb(ASN1_OP_STREAM_POST,
				&ndef_aux->val, ndef_aux->it, &sarg) <= 0)
		return 0;

	derlen = ASN1_item_ndef_i2d(ndef_aux->val, NULL, ndef_aux->it);
	p = OPENSSL_malloc(derlen);
	ndef_aux->derbuf = p;
	*pbuf = p;
	derlen = ASN1_item_ndef_i2d(ndef_aux->val, &p, ndef_aux->it);

	if (!*ndef_aux->boundary)
		return 0;
	*pbuf = *ndef_aux->boundary;
	*plen = derlen - (*ndef_aux->boundary - ndef_aux->derbuf);

	return 1;
	}