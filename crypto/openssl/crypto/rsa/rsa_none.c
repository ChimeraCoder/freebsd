
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
#include <openssl/rsa.h>
#include <openssl/rand.h>

int RSA_padding_add_none(unsigned char *to, int tlen,
	const unsigned char *from, int flen)
	{
	if (flen > tlen)
		{
		RSAerr(RSA_F_RSA_PADDING_ADD_NONE,RSA_R_DATA_TOO_LARGE_FOR_KEY_SIZE);
		return(0);
		}

	if (flen < tlen)
		{
		RSAerr(RSA_F_RSA_PADDING_ADD_NONE,RSA_R_DATA_TOO_SMALL_FOR_KEY_SIZE);
		return(0);
		}
	
	memcpy(to,from,(unsigned int)flen);
	return(1);
	}

int RSA_padding_check_none(unsigned char *to, int tlen,
	const unsigned char *from, int flen, int num)
	{

	if (flen > tlen)
		{
		RSAerr(RSA_F_RSA_PADDING_CHECK_NONE,RSA_R_DATA_TOO_LARGE);
		return(-1);
		}

	memset(to,0,tlen-flen);
	memcpy(to+tlen-flen,from,flen);
	return(tlen);
	}