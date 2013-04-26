
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
#include <time.h>

#include "cryptlib.h"

#ifndef NO_SYS_TYPES_H
# include <sys/types.h>
#endif

#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/x509.h>

#ifndef NO_ASN1_OLD

int ASN1_digest(i2d_of_void *i2d, const EVP_MD *type, char *data,
		unsigned char *md, unsigned int *len)
	{
	int i;
	unsigned char *str,*p;

	i=i2d(data,NULL);
	if ((str=(unsigned char *)OPENSSL_malloc(i)) == NULL)
		{
		ASN1err(ASN1_F_ASN1_DIGEST,ERR_R_MALLOC_FAILURE);
		return(0);
		}
	p=str;
	i2d(data,&p);

	if (!EVP_Digest(str, i, md, len, type, NULL))
		return 0;
	OPENSSL_free(str);
	return(1);
	}

#endif


int ASN1_item_digest(const ASN1_ITEM *it, const EVP_MD *type, void *asn,
		unsigned char *md, unsigned int *len)
	{
	int i;
	unsigned char *str = NULL;

	i=ASN1_item_i2d(asn,&str, it);
	if (!str) return(0);

	if (!EVP_Digest(str, i, md, len, type, NULL))
		return 0;
	OPENSSL_free(str);
	return(1);
	}