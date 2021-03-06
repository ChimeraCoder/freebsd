
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
#include <openssl/rsa.h>
#include <openssl/evp.h>

#ifndef OPENSSL_NO_FP_API
int RSA_print_fp(FILE *fp, const RSA *x, int off)
	{
	BIO *b;
	int ret;

	if ((b=BIO_new(BIO_s_file())) == NULL)
		{
		RSAerr(RSA_F_RSA_PRINT_FP,ERR_R_BUF_LIB);
		return(0);
		}
	BIO_set_fp(b,fp,BIO_NOCLOSE);
	ret=RSA_print(b,x,off);
	BIO_free(b);
	return(ret);
	}
#endif

int RSA_print(BIO *bp, const RSA *x, int off)
	{
	EVP_PKEY *pk;
	int ret;
	pk = EVP_PKEY_new();
	if (!pk || !EVP_PKEY_set1_RSA(pk, (RSA *)x))
		return 0;
	ret = EVP_PKEY_print_private(bp, pk, off, NULL);
	EVP_PKEY_free(pk);
	return ret;
	}