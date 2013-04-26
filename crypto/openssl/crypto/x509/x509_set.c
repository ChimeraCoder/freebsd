
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

int X509_set_version(X509 *x, long version)
	{
	if (x == NULL) return(0);
	if (x->cert_info->version == NULL)
		{
		if ((x->cert_info->version=M_ASN1_INTEGER_new()) == NULL)
			return(0);
		}
	return(ASN1_INTEGER_set(x->cert_info->version,version));
	}

int X509_set_serialNumber(X509 *x, ASN1_INTEGER *serial)
	{
	ASN1_INTEGER *in;

	if (x == NULL) return(0);
	in=x->cert_info->serialNumber;
	if (in != serial)
		{
		in=M_ASN1_INTEGER_dup(serial);
		if (in != NULL)
			{
			M_ASN1_INTEGER_free(x->cert_info->serialNumber);
			x->cert_info->serialNumber=in;
			}
		}
	return(in != NULL);
	}

int X509_set_issuer_name(X509 *x, X509_NAME *name)
	{
	if ((x == NULL) || (x->cert_info == NULL)) return(0);
	return(X509_NAME_set(&x->cert_info->issuer,name));
	}

int X509_set_subject_name(X509 *x, X509_NAME *name)
	{
	if ((x == NULL) || (x->cert_info == NULL)) return(0);
	return(X509_NAME_set(&x->cert_info->subject,name));
	}

int X509_set_notBefore(X509 *x, const ASN1_TIME *tm)
	{
	ASN1_TIME *in;

	if ((x == NULL) || (x->cert_info->validity == NULL)) return(0);
	in=x->cert_info->validity->notBefore;
	if (in != tm)
		{
		in=M_ASN1_TIME_dup(tm);
		if (in != NULL)
			{
			M_ASN1_TIME_free(x->cert_info->validity->notBefore);
			x->cert_info->validity->notBefore=in;
			}
		}
	return(in != NULL);
	}

int X509_set_notAfter(X509 *x, const ASN1_TIME *tm)
	{
	ASN1_TIME *in;

	if ((x == NULL) || (x->cert_info->validity == NULL)) return(0);
	in=x->cert_info->validity->notAfter;
	if (in != tm)
		{
		in=M_ASN1_TIME_dup(tm);
		if (in != NULL)
			{
			M_ASN1_TIME_free(x->cert_info->validity->notAfter);
			x->cert_info->validity->notAfter=in;
			}
		}
	return(in != NULL);
	}

int X509_set_pubkey(X509 *x, EVP_PKEY *pkey)
	{
	if ((x == NULL) || (x->cert_info == NULL)) return(0);
	return(X509_PUBKEY_set(&(x->cert_info->key),pkey));
	}