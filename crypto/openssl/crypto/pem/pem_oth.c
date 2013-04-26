
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
#include <openssl/buffer.h>
#include <openssl/objects.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/x509.h>
#include <openssl/pem.h>

/* Handle 'other' PEMs: not private keys */

void *PEM_ASN1_read_bio(d2i_of_void *d2i, const char *name, BIO *bp, void **x,
			pem_password_cb *cb, void *u)
	{
	const unsigned char *p=NULL;
	unsigned char *data=NULL;
	long len;
	char *ret=NULL;

	if (!PEM_bytes_read_bio(&data, &len, NULL, name, bp, cb, u))
		return NULL;
	p = data;
	ret=d2i(x,&p,len);
	if (ret == NULL)
		PEMerr(PEM_F_PEM_ASN1_READ_BIO,ERR_R_ASN1_LIB);
	OPENSSL_free(data);
	return(ret);
	}