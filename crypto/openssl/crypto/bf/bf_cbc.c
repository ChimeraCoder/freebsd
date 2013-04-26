
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

void BF_cbc_encrypt(const unsigned char *in, unsigned char *out, long length,
	     const BF_KEY *schedule, unsigned char *ivec, int encrypt)
	{
	register BF_LONG tin0,tin1;
	register BF_LONG tout0,tout1,xor0,xor1;
	register long l=length;
	BF_LONG tin[2];

	if (encrypt)
		{
		n2l(ivec,tout0);
		n2l(ivec,tout1);
		ivec-=8;
		for (l-=8; l>=0; l-=8)
			{
			n2l(in,tin0);
			n2l(in,tin1);
			tin0^=tout0;
			tin1^=tout1;
			tin[0]=tin0;
			tin[1]=tin1;
			BF_encrypt(tin,schedule);
			tout0=tin[0];
			tout1=tin[1];
			l2n(tout0,out);
			l2n(tout1,out);
			}
		if (l != -8)
			{
			n2ln(in,tin0,tin1,l+8);
			tin0^=tout0;
			tin1^=tout1;
			tin[0]=tin0;
			tin[1]=tin1;
			BF_encrypt(tin,schedule);
			tout0=tin[0];
			tout1=tin[1];
			l2n(tout0,out);
			l2n(tout1,out);
			}
		l2n(tout0,ivec);
		l2n(tout1,ivec);
		}
	else
		{
		n2l(ivec,xor0);
		n2l(ivec,xor1);
		ivec-=8;
		for (l-=8; l>=0; l-=8)
			{
			n2l(in,tin0);
			n2l(in,tin1);
			tin[0]=tin0;
			tin[1]=tin1;
			BF_decrypt(tin,schedule);
			tout0=tin[0]^xor0;
			tout1=tin[1]^xor1;
			l2n(tout0,out);
			l2n(tout1,out);
			xor0=tin0;
			xor1=tin1;
			}
		if (l != -8)
			{
			n2l(in,tin0);
			n2l(in,tin1);
			tin[0]=tin0;
			tin[1]=tin1;
			BF_decrypt(tin,schedule);
			tout0=tin[0]^xor0;
			tout1=tin[1]^xor1;
			l2nn(tout0,tout1,out,l+8);
			xor0=tin0;
			xor1=tin1;
			}
		l2n(xor0,ivec);
		l2n(xor1,ivec);
		}
	tin0=tin1=tout0=tout1=xor0=xor1=0;
	tin[0]=tin[1]=0;
	}