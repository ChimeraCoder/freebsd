
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

void RC5_32_set_key(RC5_32_KEY *key, int len, const unsigned char *data,
		    int rounds)
	{
	RC5_32_INT L[64],l,ll,A,B,*S,k;
	int i,j,m,c,t,ii,jj;

	if (	(rounds != RC5_16_ROUNDS) &&
		(rounds != RC5_12_ROUNDS) &&
		(rounds != RC5_8_ROUNDS))
		rounds=RC5_16_ROUNDS;

	key->rounds=rounds;
	S= &(key->data[0]);
	j=0;
	for (i=0; i<=(len-8); i+=8)
		{
		c2l(data,l);
		L[j++]=l;
		c2l(data,l);
		L[j++]=l;
		}
	ii=len-i;
	if (ii)
		{
		k=len&0x07;
		c2ln(data,l,ll,k);
		L[j+0]=l;
		L[j+1]=ll;
		}

	c=(len+3)/4;
	t=(rounds+1)*2;
	S[0]=RC5_32_P;
	for (i=1; i<t; i++)
		S[i]=(S[i-1]+RC5_32_Q)&RC5_32_MASK;

	j=(t>c)?t:c;
	j*=3;
	ii=jj=0;
	A=B=0;
	for (i=0; i<j; i++)
		{
		k=(S[ii]+A+B)&RC5_32_MASK;
		A=S[ii]=ROTATE_l32(k,3);
		m=(int)(A+B);
		k=(L[jj]+A+B)&RC5_32_MASK;
		B=L[jj]=ROTATE_l32(k,m);
		if (++ii >= t) ii=0;
		if (++jj >= c) jj=0;
		}
	}