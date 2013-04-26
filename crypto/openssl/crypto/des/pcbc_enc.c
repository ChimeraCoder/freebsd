
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

#include "des_locl.h"

void DES_pcbc_encrypt(const unsigned char *input, unsigned char *output,
		      long length, DES_key_schedule *schedule,
		      DES_cblock *ivec, int enc)
	{
	register DES_LONG sin0,sin1,xor0,xor1,tout0,tout1;
	DES_LONG tin[2];
	const unsigned char *in;
	unsigned char *out,*iv;

	in=input;
	out=output;
	iv = &(*ivec)[0];

	if (enc)
		{
		c2l(iv,xor0);
		c2l(iv,xor1);
		for (; length>0; length-=8)
			{
			if (length >= 8)
				{
				c2l(in,sin0);
				c2l(in,sin1);
				}
			else
				c2ln(in,sin0,sin1,length);
			tin[0]=sin0^xor0;
			tin[1]=sin1^xor1;
			DES_encrypt1((DES_LONG *)tin,schedule,DES_ENCRYPT);
			tout0=tin[0];
			tout1=tin[1];
			xor0=sin0^tout0;
			xor1=sin1^tout1;
			l2c(tout0,out);
			l2c(tout1,out);
			}
		}
	else
		{
		c2l(iv,xor0); c2l(iv,xor1);
		for (; length>0; length-=8)
			{
			c2l(in,sin0);
			c2l(in,sin1);
			tin[0]=sin0;
			tin[1]=sin1;
			DES_encrypt1((DES_LONG *)tin,schedule,DES_DECRYPT);
			tout0=tin[0]^xor0;
			tout1=tin[1]^xor1;
			if (length >= 8)
				{
				l2c(tout0,out);
				l2c(tout1,out);
				}
			else
				l2cn(tout0,tout1,out,length);
			xor0=tout0^sin0;
			xor1=tout1^sin1;
			}
		}
	tin[0]=tin[1]=0;
	sin0=sin1=xor0=xor1=tout0=tout1=0;
	}