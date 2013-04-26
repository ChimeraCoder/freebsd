
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
#include <openssl/rc5.h>
#include "rc5_locl.h"

void RC5_32_cbc_encrypt(const unsigned char *in, unsigned char *out,
			long length, RC5_32_KEY *ks, unsigned char *iv,
			int encrypt)
	{
	register unsigned long tin0,tin1;
	register unsigned long tout0,tout1,xor0,xor1;
	register long l=length;
	unsigned long tin[2];

	if (encrypt)
		{
		c2l(iv,tout0);
		c2l(iv,tout1);
		iv-=8;
		for (l-=8; l>=0; l-=8)
			{
			c2l(in,tin0);
			c2l(in,tin1);
			tin0^=tout0;
			tin1^=tout1;
			tin[0]=tin0;
			tin[1]=tin1;
			RC5_32_encrypt(tin,ks);
			tout0=tin[0]; l2c(tout0,out);
			tout1=tin[1]; l2c(tout1,out);
			}
		if (l != -8)
			{
			c2ln(in,tin0,tin1,l+8);
			tin0^=tout0;
			tin1^=tout1;
			tin[0]=tin0;
			tin[1]=tin1;
			RC5_32_encrypt(tin,ks);
			tout0=tin[0]; l2c(tout0,out);
			tout1=tin[1]; l2c(tout1,out);
			}
		l2c(tout0,iv);
		l2c(tout1,iv);
		}
	else
		{
		c2l(iv,xor0);
		c2l(iv,xor1);
		iv-=8;
		for (l-=8; l>=0; l-=8)
			{
			c2l(in,tin0); tin[0]=tin0;
			c2l(in,tin1); tin[1]=tin1;
			RC5_32_decrypt(tin,ks);
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
			RC5_32_decrypt(tin,ks);
			tout0=tin[0]^xor0;
			tout1=tin[1]^xor1;
			l2cn(tout0,tout1,out,l+8);
			xor0=tin0;
			xor1=tin1;
			}
		l2c(xor0,iv);
		l2c(xor1,iv);
		}
	tin0=tin1=tout0=tout1=xor0=xor1=0;
	tin[0]=tin[1]=0;
	}

void RC5_32_encrypt(unsigned long *d, RC5_32_KEY *key)
	{
	RC5_32_INT a,b,*s;

	s=key->data;

	a=d[0]+s[0];
	b=d[1]+s[1];
	E_RC5_32(a,b,s, 2);
	E_RC5_32(a,b,s, 4);
	E_RC5_32(a,b,s, 6);
	E_RC5_32(a,b,s, 8);
	E_RC5_32(a,b,s,10);
	E_RC5_32(a,b,s,12);
	E_RC5_32(a,b,s,14);
	E_RC5_32(a,b,s,16);
	if (key->rounds == 12)
		{
		E_RC5_32(a,b,s,18);
		E_RC5_32(a,b,s,20);
		E_RC5_32(a,b,s,22);
		E_RC5_32(a,b,s,24);
		}
	else if (key->rounds == 16)
		{
		/* Do a full expansion to avoid a jump */
		E_RC5_32(a,b,s,18);
		E_RC5_32(a,b,s,20);
		E_RC5_32(a,b,s,22);
		E_RC5_32(a,b,s,24);
		E_RC5_32(a,b,s,26);
		E_RC5_32(a,b,s,28);
		E_RC5_32(a,b,s,30);
		E_RC5_32(a,b,s,32);
		}
	d[0]=a;
	d[1]=b;
	}

void RC5_32_decrypt(unsigned long *d, RC5_32_KEY *key)
	{
	RC5_32_INT a,b,*s;

	s=key->data;

	a=d[0];
	b=d[1];
	if (key->rounds == 16) 
		{
		D_RC5_32(a,b,s,32);
		D_RC5_32(a,b,s,30);
		D_RC5_32(a,b,s,28);
		D_RC5_32(a,b,s,26);
		/* Do a full expansion to avoid a jump */
		D_RC5_32(a,b,s,24);
		D_RC5_32(a,b,s,22);
		D_RC5_32(a,b,s,20);
		D_RC5_32(a,b,s,18);
		}
	else if (key->rounds == 12)
		{
		D_RC5_32(a,b,s,24);
		D_RC5_32(a,b,s,22);
		D_RC5_32(a,b,s,20);
		D_RC5_32(a,b,s,18);
		}
	D_RC5_32(a,b,s,16);
	D_RC5_32(a,b,s,14);
	D_RC5_32(a,b,s,12);
	D_RC5_32(a,b,s,10);
	D_RC5_32(a,b,s, 8);
	D_RC5_32(a,b,s, 6);
	D_RC5_32(a,b,s, 4);
	D_RC5_32(a,b,s, 2);
	d[0]=a-s[0];
	d[1]=b-s[1];
	}