
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

#include <openssl/rc5.h>
#include "rc5_locl.h"
#include <openssl/opensslv.h>

const char RC5_version[]="RC5" OPENSSL_VERSION_PTEXT;

void RC5_32_ecb_encrypt(const unsigned char *in, unsigned char *out,
			RC5_32_KEY *ks, int encrypt)
	{
	unsigned long l,d[2];

	c2l(in,l); d[0]=l;
	c2l(in,l); d[1]=l;
	if (encrypt)
		RC5_32_encrypt(d,ks);
	else
		RC5_32_decrypt(d,ks);
	l=d[0]; l2c(l,out);
	l=d[1]; l2c(l,out);
	l=d[0]=d[1]=0;
	}