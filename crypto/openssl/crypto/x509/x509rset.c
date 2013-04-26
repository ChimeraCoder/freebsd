
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
#include <openssl/asn1.h>
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/x509.h>

int X509_REQ_set_version(X509_REQ *x, long version)
	{
	if (x == NULL) return(0);
	return(ASN1_INTEGER_set(x->req_info->version,version));
	}

int X509_REQ_set_subject_name(X509_REQ *x, X509_NAME *name)
	{
	if ((x == NULL) || (x->req_info == NULL)) return(0);
	return(X509_NAME_set(&x->req_info->subject,name));
	}

int X509_REQ_set_pubkey(X509_REQ *x, EVP_PKEY *pkey)
	{
	if ((x == NULL) || (x->req_info == NULL)) return(0);
	return(X509_PUBKEY_set(&x->req_info->pubkey,pkey));
	}