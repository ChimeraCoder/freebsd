
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

int X509_CRL_set_version(X509_CRL *x, long version)
	{
	if (x == NULL) return(0);
	if (x->crl->version == NULL)
		{
		if ((x->crl->version=M_ASN1_INTEGER_new()) == NULL)
			return(0);
		}
	return(ASN1_INTEGER_set(x->crl->version,version));
	}

int X509_CRL_set_issuer_name(X509_CRL *x, X509_NAME *name)
	{
	if ((x == NULL) || (x->crl == NULL)) return(0);
	return(X509_NAME_set(&x->crl->issuer,name));
	}


int X509_CRL_set_lastUpdate(X509_CRL *x, const ASN1_TIME *tm)
	{
	ASN1_TIME *in;

	if (x == NULL) return(0);
	in=x->crl->lastUpdate;
	if (in != tm)
		{
		in=M_ASN1_TIME_dup(tm);
		if (in != NULL)
			{
			M_ASN1_TIME_free(x->crl->lastUpdate);
			x->crl->lastUpdate=in;
			}
		}
	return(in != NULL);
	}

int X509_CRL_set_nextUpdate(X509_CRL *x, const ASN1_TIME *tm)
	{
	ASN1_TIME *in;

	if (x == NULL) return(0);
	in=x->crl->nextUpdate;
	if (in != tm)
		{
		in=M_ASN1_TIME_dup(tm);
		if (in != NULL)
			{
			M_ASN1_TIME_free(x->crl->nextUpdate);
			x->crl->nextUpdate=in;
			}
		}
	return(in != NULL);
	}

int X509_CRL_sort(X509_CRL *c)
	{
	int i;
	X509_REVOKED *r;
	/* sort the data so it will be written in serial
	 * number order */
	sk_X509_REVOKED_sort(c->crl->revoked);
	for (i=0; i<sk_X509_REVOKED_num(c->crl->revoked); i++)
		{
		r=sk_X509_REVOKED_value(c->crl->revoked,i);
		r->sequence=i;
		}
	c->crl->enc.modified = 1;
	return 1;
	}

int X509_REVOKED_set_revocationDate(X509_REVOKED *x, ASN1_TIME *tm)
	{
	ASN1_TIME *in;

	if (x == NULL) return(0);
	in=x->revocationDate;
	if (in != tm)
		{
		in=M_ASN1_TIME_dup(tm);
		if (in != NULL)
			{
			M_ASN1_TIME_free(x->revocationDate);
			x->revocationDate=in;
			}
		}
	return(in != NULL);
	}

int X509_REVOKED_set_serialNumber(X509_REVOKED *x, ASN1_INTEGER *serial)
	{
	ASN1_INTEGER *in;

	if (x == NULL) return(0);
	in=x->serialNumber;
	if (in != serial)
		{
		in=M_ASN1_INTEGER_dup(serial);
		if (in != NULL)
			{
			M_ASN1_INTEGER_free(x->serialNumber);
			x->serialNumber=in;
			}
		}
	return(in != NULL);
	}