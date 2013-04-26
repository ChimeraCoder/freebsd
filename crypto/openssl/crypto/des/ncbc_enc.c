
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

#ifdef CBC_ENC_C__DONT_UPDATE_IV
void DES_cbc_encrypt(const unsigned char *in, unsigned char *out, long length,
		     DES_key_schedule *_schedule, DES_cblock *ivec, int enc)
#else
void DES_ncbc_encrypt(const unsigned char *in, unsigned char *out, long length,
		     DES_key_schedule *_schedule, DES_cblock *ivec, int enc)
#endif
	{
	register DES_LONG tin0,tin1;
	register DES_LONG tout0,tout1,xor0,xor1;
	register long l=length;
	DES_LONG tin[2];
	unsigned char *iv;

	iv = &(*ivec)[0];

	if (enc)
		{
		c2l(iv,tout0);
		c2l(iv,tout1);
		for (l-=8; l>=0; l-=8)
			{
			c2l(in,tin0);
			c2l(in,tin1);
			tin0^=tout0; tin[0]=tin0;
			tin1^=tout1; tin[1]=tin1;
			DES_encrypt1((DES_LONG *)tin,_schedule,DES_ENCRYPT);
			tout0=tin[0]; l2c(tout0,out);
			tout1=tin[1]; l2c(tout1,out);
			}
		if (l != -8)
			{
			c2ln(in,tin0,tin1,l+8);
			tin0^=tout0; tin[0]=tin0;
			tin1^=tout1; tin[1]=tin1;
			DES_encrypt1((DES_LONG *)tin,_schedule,DES_ENCRYPT);
			tout0=tin[0]; l2c(tout0,out);
			tout1=tin[1]; l2c(tout1,out);
			}
#ifndef CBC_ENC_C__DONT_UPDATE_IV
		iv = &(*ivec)[0];
		l2c(tout0,iv);
		l2c(tout1,iv);
#endif
		}
	else
		{
		c2l(iv,xor0);
		c2l(iv,xor1);
		for (l-=8; l>=0; l-=8)
			{
			c2l(in,tin0); tin[0]=tin0;
			c2l(in,tin1); tin[1]=tin1;
			DES_encrypt1((DES_LONG *)tin,_schedule,DES_DECRYPT);
			tout0=tin[0]^xor0;
			tout1=tin[1]^xor1;
			l2c(tout0,out);
			l2c(tout1,out);
			xor0=tin0;
			xor1=tin1;
			}
		if (l != -8)
			{
			c2l(in,tin0); tin[0]=tin0;
			c2l(in,tin1); tin[1]=tin1;
			DES_encrypt1((DES_LONG *)tin,_schedule,DES_DECRYPT);
			tout0=tin[0]^xor0;
			tout1=tin[1]^xor1;
			l2cn(tout0,tout1,out,l+8);
#ifndef CBC_ENC_C__DONT_UPDATE_IV
			xor0=tin0;
			xor1=tin1;
#endif
			}
#ifndef CBC_ENC_C__DONT_UPDATE_IV 
		iv = &(*ivec)[0];
		l2c(xor0,iv);
		l2c(xor1,iv);
#endif
		}
	tin0=tin1=tout0=tout1=xor0=xor1=0;
	tin[0]=tin[1]=0;
	}