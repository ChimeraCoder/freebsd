
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

#include <openssl/blowfish.h>
#include "bf_locl.h"
#include <openssl/opensslv.h>

/* Blowfish as implemented from 'Blowfish: Springer-Verlag paper'
 * (From LECTURE NOTES IN COMPUTER SCIENCE 809, FAST SOFTWARE ENCRYPTION,
 * CAMBRIDGE SECURITY WORKSHOP, CAMBRIDGE, U.K., DECEMBER 9-11, 1993)
 */

const char BF_version[]="Blowfish" OPENSSL_VERSION_PTEXT;

const char *BF_options(void)
	{
#ifdef BF_PTR
	return("blowfish(ptr)");
#elif defined(BF_PTR2)
	return("blowfish(ptr2)");
#else
	return("blowfish(idx)");
#endif
	}

void BF_ecb_encrypt(const unsigned char *in, unsigned char *out,
	     const BF_KEY *key, int encrypt)
	{
	BF_LONG l,d[2];

	n2l(in,l); d[0]=l;
	n2l(in,l); d[1]=l;
	if (encrypt)
		BF_encrypt(d,key);
	else
		BF_decrypt(d,key);
	l=d[0]; l2n(l,out);
	l=d[1]; l2n(l,out);
	l=d[0]=d[1]=0;
	}