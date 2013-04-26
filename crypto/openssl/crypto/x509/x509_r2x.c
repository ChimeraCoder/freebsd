
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

#include <stdio.h>
#include "cryptlib.h"
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/asn1.h>
#include <openssl/x509.h>
#include <openssl/objects.h>
#include <openssl/buffer.h>

X509 *X509_REQ_to_X509(X509_REQ *r, int days, EVP_PKEY *pkey)
	{
	X509 *ret=NULL;
	X509_CINF *xi=NULL;
	X509_NAME *xn;

	if ((ret=X509_new()) == NULL)
		{
		X509err(X509_F_X509_REQ_TO_X509,ERR_R_MALLOC_FAILURE);
		goto err;
		}

	/* duplicate the request */
	xi=ret->cert_info;

	if (sk_X509_ATTRIBUTE_num(r->req_info->attributes) != 0)
		{
		if ((xi->version=M_ASN1_INTEGER_new()) == NULL) goto err;
		if (!ASN1_INTEGER_set(xi->version,2)) goto err;
/*		xi->extensions=ri->attributes; <- bad, should not ever be done
		ri->attributes=NULL; */
		}

	xn=X509_REQ_get_subject_name(r);
	if (X509_set_subject_name(ret,X509_NAME_dup(xn)) == 0)
		goto err;
	if (X509_set_issuer_name(ret,X509_NAME_dup(xn)) == 0)
		goto err;

	if (X509_gmtime_adj(xi->validity->notBefore,0) == NULL)
		goto err;
	if (X509_gmtime_adj(xi->validity->notAfter,(long)60*60*24*days) == NULL)
		goto err;

	X509_set_pubkey(ret,X509_REQ_get_pubkey(r));

	if (!X509_sign(ret,pkey,EVP_md5()))
		goto err;
	if (0)
		{
err:
		X509_free(ret);
		ret=NULL;
		}
	return(ret);
	}